#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// deletes spaces in the task
void remove_spaces(char* source) {
    char* i = source;
    char* j = source;
    while (*j != '\0') {
        *i = *j++;
        if (*i != ' ')
            i++;
    }
    *i = '\0';
}

unsigned short task_solver(char msg[], int msg_len)
{
    // possible tasks:
    // a x b
    // a x b x c
    // (a x b) x c
    // a x (b x c)

    unsigned short i = 0;  // current pos
    bool bracket_open = false, bracket_closed = false;
    bool bracket_pos1 = false, bracket_pos2 = false, bracket_pos3 = false, bracket_pos4 = false;
    //  a x b x c
    // 1   2 3   4

    if (msg_len == 0)
    {
        // printf("Error: msg len is 0.\n");
        return 1;
    }

    if (msg_len > 31)
    {
        // printf("Error: msg length is more than 31.\n");
        return 1;
    }

    if (msg[i] == '(') {
        bracket_open = true;
        bracket_pos1 = true;
        i++;
    }

    // declaring num1
    bool num1_started = false;
    bool num1_point = false;

    // if msg[i] is a number
    if (i < msg_len && '0' <= msg[i] && msg[i] <= '9')
    {
        num1_started = true;
        while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
            i++;
        }
    }
    else
    {
        // printf("Error: starts not with a wrong symbol or too short.\n");
        return 1;
    }


    if (i >= msg_len) {
        // printf("Error: first number ends too early.\n");
        return 1;
    }

    if (msg[i] != '.' && msg[i] != '+' && msg[i] != '-' && msg[i] != '*' && msg[i] != '/')
    {
        // printf("Error: wrong symbol after first number (123x).\n");
        return 1;
    }
    if (msg[i] == '.')
    {
        num1_point = true;
        i++;
        // continue collecting num1 after point
        while (i < msg_len)
        {
            if ('0' <= msg[i] && msg[i] <= '9')
            {
                i++;
            }
            else if (msg[i] == '+' || msg[i] == '-' || msg[i] == '*' || msg[i] == '/')
                break;
            else  // not a number and not a mark
            {
                // printf("Error: wrong symbol after num1 (12.34x).\n");
                return 1;
            }

        }
        // num1 has ended
    }

    if (i >= msg_len) {
        // printf("Error: too short task (12.34).\n");
        return 1;
    }
    if (msg[i] != '+' && msg[i] != '-' && msg[i] != '*' && msg[i] != '/')
    {
        // printf("Error: symbol is not a operator after num1 (12.34x).\n");
        return 1;
    }

    // writing an operator
    i++;
    if (i >= msg_len) {
        // printf("Error: too short task (12.34*).\n");
        return 1;
    }

    if (msg[i] == '(')
    {
        if (bracket_open == true) {
            // printf("Error: two opened brackets ( (( ).\n");
            return 1;
        }
        bracket_open = true;
        bracket_pos2 = true;

        i++;
        if (i >= msg_len) {
            // printf("Error: too short task ( 12.34*( ).\n");
            return 1;
        }
    }

    if ('0' > msg[i] || msg[i] > '9')
    {
        // printf("Error: not a number after operator (12.34+x).\n");
        return 1;
    }

    // declaring num2
    bool num2_started = false;
    bool num2_point = false;
    num2_started = true; // msg[i] is 100% a number and i < msg_len (has been checked earlier)

    // writing while numbers
    while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
        i++;
    }

    if (i >= msg_len) {
        if (bracket_open == false) {  // if there are no brackets then it has format "1.0+2" correct
            // printf("[Format validation is OK]\n");
            return 0;
        }
        else
        {
            // printf("Error: too short ( (1.0+2) ).\n");
            return 1;
        }
    }

    if (msg[i] != ')' && msg[i] != '.' && msg[i] != '+' && msg[i] != '-' && msg[i] != '*' && msg[i] != '/')
    {
        // printf("Error: wrong symbol after second number (12.0+12x).\n");
        return 1;
    }

    if (msg[i] == ')')
    {
        if (bracket_open == true)
        {
            bracket_pos3 = true;
            bracket_closed = true;
            i++;
            if (i >= msg_len) {
                // printf("Error: can't use such brackets format ( (1.2+2) ).\n");
                return 1;
            }
        }
        else
        {
            // printf("Error: closing brackets without opening ( 1.2+0.2) ).\n");
            return 1;
        }
    }
    else if (msg[i] == '.')
    {
        num2_point = true;
        i++;
        if (i >= msg_len) {
            // printf("Error: ends after point (12.0+12.).\n");
            return 1;
        }

        // continue collecting num2 after point
        while (i < msg_len)
        {
            if ('0' <= msg[i] && msg[i] <= '9')
            {
                i++;
            }
            else if (msg[i] == '+' || msg[i] == '-' || msg[i] == '*' || msg[i] == '/')
                break;
            else if (msg[i] == ')')
            {
                if (bracket_open == true)
                {
                    bracket_pos3 = true;
                    bracket_closed = true;
                    i++;
                    if (i >= msg_len) {
                        // printf("Error: can't use such brackets format ( (1.2+0.2) ).\n");
                        return 1;
                    }
                    break;
                }
                else
                {
                    // printf("Error: closing brackets without opening ( 1.2+0.2) ).\n");
                    return 1;
                }
            }
            else  // not a number, not a mark, not a ')'
            {
                // printf("Error: wrong symbol after num2 (1.2+3.4x).\n");
                return 1;
            }

        }
        // num2 has ended
    }

    if (i >= msg_len)
    {
        if (bracket_open == true)
        {
            // printf("Error: bracket is not closed ( (1.2+3.4 ).\n");
            return 1;
        }
        // printf("[Format validation is OK]\n");
        return 0;
    }
    if (msg[i] != '+' && msg[i] != '-' && msg[i] != '*' && msg[i] != '/')
    {
        // printf("Error: wrong symbol after num2 or num2).\n");
        return 1;
    }

    // writing an operator
    i++;
    if (i >= msg_len) {
        // printf("Error: too short task (1.2+3.4+).\n");
        return 1;
    }

    if ('0' < msg[i] && msg[i] > '9')
    {
        // printf("Error: not a number after operator (1.2+3.4+x).\n");
        return 1;
    }

    // declaring num3
    bool num3_started = false;
    bool num3_point = false;
    num3_started = true;  // msg[i] is 100% a number and i < msg_len (has been checked earlier)

    // writing while numbers
    while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
        i++;
    }

    if (i >= msg_len)
    {
        if (bracket_closed == true || bracket_open == false)
        {
            // printf("[Format validation is OK]\n");
            return 0;
        }
        else
        {
            // printf("Error: wrong brackets format (e.g. not closed).\n");
            return 1;
        }
    }

    if (msg[i] == ')')
    {
        if (bracket_open == true && bracket_closed == false && bracket_pos1 == false)
        {
            bracket_pos4 = true;
            i++;
            if (msg_len != i) {
                // printf("Error: symbols after last close bracket ( (1+2+3)x ).\n");
                return 1;
            }
            // printf("[Format validation is OK]\n");
            return 0;
        }
        else {
            // printf("Error: wrong close bracket (not opened or already closed).\n");
            return 1;
        }
        i++;
    }

    if (msg[i] == '.')
    {
        num3_point = true;
        i++;
        if (i >= msg_len) {
            // printf("Error: ends after point (1.2+3.4+5.).\n");
            return 1;
        }

        // continue collecting num3 after point
        while (i < msg_len)
        {
            if ('0' <= msg[i] && msg[i] <= '9')
            {
                i++;
            }
            else if (msg[i] == ')')
            {
                if (bracket_open == true && bracket_closed == false && bracket_pos1 == false)
                {
                    bracket_pos4 = true;
                    i++;
                    if (msg_len != i) {
                        // printf("Error: symbols after last close bracket ( (1+2+3)x ).\n");
                        return 1;
                    }
                    // printf("[Format validation is OK]\n");
                    return 0;
                }
                else {
                    // printf("Error: wrong close bracket (not opened or already closed).\n");
                    return 1;
                }
            }
            else  // not a number, not a ')'
            {
                // printf("Error: wrong symbol after num3 (1.2+3.4+5.6x).\n");
                return 1;
            }

        }
        // num3 has ended
    }

    if (i >= msg_len)
    {
        if ((bracket_open == false && bracket_closed == false) || (bracket_open == true && bracket_closed == true))
        {
            // printf("[Format validation is OK]\n");
            return 0;
        }
        else
        {
            // printf("Error: wrong brackets format.\n");
            return 1;
        }
    }
    else
    {
        // printf("Error: wrong symbols after the task ( (1+2+3)x ).\n");
        return 1;
    }
}

int main() {
    char msg[100];
    // printf("Validation test.\n");
    // printf("Enter the task: ");
    fgets(msg, sizeof(msg), stdin);

    msg[strcspn(msg, "\n")] = '\0';

    remove_spaces(msg);
    int msg_len = strlen(msg);

    short int errors = task_solver(msg, msg_len);
    if (errors == 1)
        printf("WA");  // Wrong Answer
    else
        printf("OK");  // Correct

    return 0;
}
