#!/bin/zsh

git grep --color=always $@ \
    | fzf \
    --ansi \
    --height 40 \
    --reverse \
    --multi \
    --preview="echo {} | awk -F: '{print \$1}' | xargs cat" \
    --preview-window=right:60% \
    | awk -F: '{print $1}'
