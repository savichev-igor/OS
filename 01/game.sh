#!/usr/local/bin/bash

input_pipe=/tmp/saviPipe_1
output_pipe=/tmp/saviPipe_2

init_game() {
    trap "rm -f $input_pipe" exit

    if [[ ! -p $input_pipe ]]; then
        waiting=true
        mkfifo $input_pipe
    else
        trap "rm -f $output_pipe" exit

        if [[ ! -p $output_pipe ]]; then
            waiting=false
            mkfifo $output_pipe
        fi

        # Т.к. мы второй игрок, надо поменять пайпы местами
        local tmp=$input_pipe
        input_pipe=$output_pipe
        output_pipe=$tmp
    fi
}

terminal_rows=$(tput lines)
terminal_cols=$(tput cols)
size_field=$((terminal_cols / 6))
init_offset_rows=$(( (terminal_rows - size_field) / 2 ))
init_offset_cols=$(( (terminal_cols - size_field) / 2 ))

set_cursor() {
    tput cup $1 $2
}

init_field() {
    clear

    local start_offset_rows=$init_offset_rows
    local start_offset_cols=$init_offset_cols
    offset_draw=$((size_field / 3))
    set_cursor $start_offset_rows $start_offset_cols $size_field

    for (( i=1; i <= $size_field - 1; i++ )); do
        for (( j=1; j <= $size_field - 1; j++ )); do
            if [[ $i -eq 1 ]] || 
                [[ $i -eq $((offset_draw)) ]] ||
                [[ $i -eq $((offset_draw * 2)) ]] ||
                [[ $i -eq $((offset_draw * 3)) ]]; then
                echo -n "*"
            fi
        done
        echo "*"
        ((start_offset_cols+=offset_draw))
        set_cursor $start_offset_rows $start_offset_cols $size_field
        echo -n "*"
        ((start_offset_cols+=offset_draw))
        set_cursor $start_offset_rows $start_offset_cols $size_field
        echo -n "*"
        ((start_offset_cols+=offset_draw))
        set_cursor $start_offset_rows $start_offset_cols $size_field
        echo -n "*"

        ((start_offset_cols-=offset_draw * 3))
        ((start_offset_rows++))
        set_cursor $start_offset_rows $start_offset_cols $size_field
    done
}

cur_line=0
current_moves_x=()
current_moves_o=()

do_move() {
    local move=x_${1}_${2}
    local move_x="${move}[0]"
    local move_y="${move}[1]"
    set_cursor ${!move_x} ${!move_y}

    if $3; then
        echo 'X'
        current_moves_x+=($move)
    else
        echo 'O'
        current_moves_o+=($move)
    fi

    set_cursor $cur_line 0
}

# Здесь мы переберём все выигрышные комбинации и остановимся, когда найдём победную, если такая есть
check_diff_arrays() {
    local current_moves=$@
    local second=${current_moves[*]}
    for i in {1..8}; do
        local tmp_str=win_$i
        local win="${tmp_str}[@]"
        local result=()
        for item in ${!win}; do
            if [[ ! $second =~ $item ]]; then
                result+=($item)
            fi
        done
        check_win=${result[@]}
        if [[ -z $check_win ]]; then
            break;
        fi
    done
}

init_game
init_field
set_cursor 0 0

x_0_0=( $((init_offset_rows + offset_draw / 2)) $((init_offset_cols + offset_draw / 2)) )
x_0_1=( $((init_offset_rows + offset_draw / 2)) $((init_offset_cols + offset_draw + offset_draw / 2)) )
x_0_2=( $((init_offset_rows + offset_draw / 2)) $((init_offset_cols + (offset_draw * 2) + offset_draw / 2)) )

x_1_0=( $((init_offset_rows + offset_draw + offset_draw / 2)) $((init_offset_cols + offset_draw / 2)) )
x_1_1=( $((init_offset_rows + offset_draw + offset_draw / 2)) $((init_offset_cols + offset_draw + offset_draw / 2)) )
x_1_2=( $((init_offset_rows + offset_draw + offset_draw / 2)) $((init_offset_cols + (offset_draw * 2) + offset_draw / 2)) )

x_2_0=( $((init_offset_rows + (offset_draw * 2) + offset_draw / 2)) $((init_offset_cols + offset_draw / 2)) )
x_2_1=( $((init_offset_rows + (offset_draw * 2) + offset_draw / 2)) $((init_offset_cols + offset_draw + offset_draw / 2)) )
x_2_2=( $((init_offset_rows + (offset_draw * 2) + offset_draw / 2)) $((init_offset_cols + (offset_draw * 2) + offset_draw / 2)) )

win_1=(x_0_0 x_0_1 x_0_2)
win_2=(x_1_0 x_1_1 x_1_2)
win_3=(x_2_0 x_2_1 x_2_2)

win_4=(x_0_0 x_1_0 x_2_0)
win_5=(x_0_1 x_1_1 x_2_1)
win_6=(x_0_2 x_1_2 x_2_2)

win_7=(x_0_0 x_1_1 x_2_2)
win_8=(x_0_2 x_1_1 x_2_0)

game_logic() {
    do_move $a $b $2
    if $2; then
        check_diff_arrays ${current_moves_x[@]}
    else
        check_diff_arrays ${current_moves_o[@]}
    fi
    if [[ -z $check_win ]]; then
        echo $1
        sleep 3
        clear
        exit
    fi
    waiting=$2
}

while true
    do
        if $waiting; then
            stty -echo
            if read a b < $input_pipe; then
                game_logic 'YOU LOST !' false
            fi
        else
            stty echo
            echo 'Your move:'
            cur_line=$((cur_line + 2))
            if read a b; then
                if ! [[ $a =~ ^[0-9]+$ ]] || ! [[ $b =~ ^[0-9]+$ ]] ||
                    [[ $a -gt 2 ]] || [[ $a -lt 0 ]] ||
                    [[ $b -gt 2 ]] || [[ $b -lt 0 ]]; then
                    echo 'Wrong move ! Try again...'
                    cur_line=$((cur_line + 1))
                else
                    echo $a $b > $output_pipe
                    game_logic 'YOU WIN !' true
                fi
            fi
        fi
    done
