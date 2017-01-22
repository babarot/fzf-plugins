#!/bin/zsh

FZF_DEFAULT_OPTS=
FZF_DEFAULT_OPTS+=" --bind 'ctrl-i:toggle-out'"
FZF_DEFAULT_OPTS+=" --bind 'down:preview-down,up:preview-up'"
FZF_DEFAULT_OPTS+=" --bind 'ctrl-y:execute(printf $PWD/{} | pbcopy)'"
FZF_DEFAULT_OPTS+=" --bind '?:toggle-preview'"

is_git_repo() {
    git rev-parse --is-inside-work-tree &>/dev/null
    return $?
}

get_git_branch() {
    is_git_repo && git rev-parse --abbrev-ref HEAD
}

get_lines() {
    tput lines
}

get_cursor_pos() {
    # based on a script from http://invisible-island.net/xterm/xterm.faq.html
    exec </dev/tty
    oldstty=$(stty -g)
    stty raw -echo min 0
    echo -en "\033[6n" >/dev/tty
    IFS=';' read -r -d R -a pos
    stty $oldstty
    # change from one-based to zero based so they work with: tput cup $row $col
    echo $((${pos[0]:2} - 1)) # strip off the esc-[
}

branch="$(get_git_branch)"

viewer=$(cat <<SHELL
if file {} | grep -q text; then
    # pygmentize -g {} 2>/dev/null || less {}
    highlight -O ansi -l {} 2>/dev/null || less {}
else
    file {}
fi
SHELL
)

diff_files() {
    if [[ $branch == 'master' ]]; then
        finder
        return $?
    fi

    {
        echo -e "* $branch"
        echo    "  ================"
        git diff \
            --name-only \
            "origin/master...$branch"
    } |
    fzf \
        --header-lines=2 \
        --ansi \
        --preview="git diff --color origin/master {}" \
        --preview-window="$size" \
        --bind "enter:execute-multi(vim -p {} </dev/tty >/dev/tty)" "$@"
}

finder() {
    fzf \
        --ansi \
        --preview="$viewer" \
        --preview-window="$size" \
        --bind "enter:execute-multi(vim -p {})" "$@"
}

do_help() {
    cat <<HELP >&2
    $ re [-f]
HELP
}

main() {
    local is_finder=false
    local -a opts args

    while (( $# > 0 ))
    do
        case "$1" in
            -h|--help)
                do_help
                return 1
                ;;
            -f)
                is_finder=true
                ;;
            -*)
                opts+=("$1")
                ;;
            "")
                ;;
            *)
                args+=("$1")
                ;;
        esac
        shift
    done

    if (( COLUMNS > LINES )); then
        size="right:60%"
    else
        size="up:70%"
    fi

    if (( $(get_cursor_pos) < ($(get_lines)/2) )); then
        opts+=("--reverse")
    fi

    if (( ${#args[@]} > 0 )); then
        pt -l ${args[0]} 2>/dev/null \
            | fzf \
            --preview "grep -n --color=always ${args[0]} {}" \
            --preview-window="$size" \
            --bind "enter:execute-multi(vim -p {} </dev/tty >/dev/tty)" "$@"
    else
        if ! $is_finder && is_git_repo; then
            diff_files "${opts[@]}"
        else
            finder "${opts[@]}"
        fi
    fi
}

main ${1:+"$@"}
