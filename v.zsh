#!/bin/zsh

viewer=$(cat <<SHELL
if file {} | grep -q text; then
    pygmentize {} 2>/dev/null || less {}
else
    file {} \
        | sed -E "s/^([^:]*)(:.*)$/$fg[yellow]\1$reset_color\2/g"
fi
SHELL
)

while read f
do
    [[ -f $f ]] && echo $f
done <~/.vim_mru_files |
fzf \
    --ansi \
    --preview="$viewer" \
    --preview-window=up:75% \
    --bind "enter:execute(vim {})"
