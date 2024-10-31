#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct SolverData {
    double result;
    unsigned short error;
};

// delete spaces in the task
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

// calculate task with two numbers
struct SolverData calc_two_nums(double num1, double num2, char oper) {
    struct SolverData sd;
    sd.result = 0;
    sd.error = 0;

    if (oper == '+')
        sd.result = num1 + num2;
    if (oper == '-')
        sd.result = num1 - num2;
    if (oper == '*')
        sd.result = num1 * num2;
    if (oper == '/') {
        if (num2 == 0) {
            sd.error = 2;  // Division by zero
        }
        else
            sd.result = num1 / num2;
    }

    return sd;
}

struct SolverData calc_three_nums(double n1, double n2, double n3, char o1, char o2, bool bp1, bool bp2, bool bp3, bool bp4)
{
    //  a x b x c
    // 1   2 3   4
    // possible options: 13, 24

    struct SolverData sd;
    sd.error = 0;

    if (bp1 == false && bp2 == false && bp3 == false && bp4 == false)
    {
        if ((o1 != '*' && o1 != '/') && (o2 == '*' || o2 == '/'))  // o2 then o1
        {
            sd = calc_two_nums(n2, n3, o2);
            if (sd.error != 0)
                return sd;
            double temp_res = sd.result;
            sd = calc_two_nums(n1, temp_res, o1);
            return sd;  // whatever error
        }
        else  // other cases where o1 then o2
        {
            sd = calc_two_nums(n1, n2, o1);
            if (sd.error != 0)
                return sd;
            double temp_res = sd.result;
            sd = calc_two_nums(temp_res, n3, o2);
            return sd;  // whatever error
        }
    }

    if (bp1 == true && bp2 == false && bp3 == true && bp4 == false)
    {
        sd = calc_two_nums(n1, n2, o1);
        if (sd.error != 0)
            return sd;
        double temp_res = sd.result;
        sd = calc_two_nums(temp_res, n3, o2);
        return sd;  // whatever error
    }

    if (bp1 == false && bp2 == true && bp3 == false && bp4 == true)
    {
        sd = calc_two_nums(n2, n3, o2);
        if (sd.error != 0)
            return sd;
        double temp_res = sd.result;
        sd = calc_two_nums(n1, temp_res, o1);
        return sd;  // whatever error
    }

    sd.error = 1;  // Wrong format
    printf("Error: wrong brackets positions.\n");
    return sd;
}

struct SolverData task_solver(char msg[], int msg_len)
{
    // possible tasks:
    // a x b
    // a x b x c
    // (a x b) x c
    // a x (b x c)

    struct SolverData solver_data;
    solver_data.result = 0;
    solver_data.error = 0;

    unsigned short i = 0;  // current pos
    bool bracket_open = false, bracket_closed = false;
    bool bracket_pos1 = false, bracket_pos2 = false, bracket_pos3 = false, bracket_pos4 = false;
    //  a x b x c
    // 1   2 3   4

    if (msg_len == 0)
    {
        solver_data.error = 1;  // Wrong format
        printf("Error: msg_len==0.\n");
        return solver_data;
    }

    if (msg[i] == '(') {
        bracket_open = true;
        bracket_pos1 = true;
        i++;
    }

    // declaring num1
    bool num1_started = false;
    bool num1_point = false;
    int decimal_amount = 1;
    double num1 = 0;

    num1_started = true;
    // writing while numbers
    while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
        num1 = num1 * 10 + (msg[i] - '0');
        i++;
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
                num1 = num1 * 10 + (msg[i] - '0');
                decimal_amount *= 10;
                i++;
            }
            else if (msg[i] == '+' || msg[i] == '-' || msg[i] == '*' || msg[i] == '/')
                break;
        }
        num1 = num1 / decimal_amount;  // num1 has ended
    }

    // writing an operator
    char oper1;
    oper1 = msg[i];
    i++;

    if (msg[i] == '(')
    {
        bracket_open = true;
        bracket_pos2 = true;
        i++;
    }

    // declaring num2
    bool num2_started = false;
    bool num2_point = false;
    decimal_amount = 1;
    double num2 = 0;

    // msg[i] is 100% a number and i < msg_len (has been checked earlier)
    num2_started = true;
    // writing while numbers
    while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
        num2 = num2 * 10 + (msg[i] - '0');
        i++;
    }

    if (i >= msg_len) {
        solver_data = calc_two_nums(num1, num2, oper1);
        if (solver_data.error == 2) {
            printf("Error: Division by zero (num1 / num2).\n");
        }
        return solver_data;
    }

    if (msg[i] == ')')
    {
        bracket_pos3 = true;
        bracket_closed = true;
        i++;
    }
    else if (msg[i] == '.')
    {
        num2_point = true;
        i++;

        // continue collecting num2 after point
        while (i < msg_len)
        {
            if ('0' <= msg[i] && msg[i] <= '9')
            {
                num2 = num2 * 10 + (msg[i] - '0');
                decimal_amount *= 10;
                i++;
            }
            else if (msg[i] == '+' || msg[i] == '-' || msg[i] == '*' || msg[i] == '/')
                break;
            else if (msg[i] == ')')
            {
                bracket_pos3 = true;
                bracket_closed = true;
                i++;
                break;
            }
        }
        num2 = num2 / decimal_amount;  // num2 has ended
    }

    if (i >= msg_len)
    {
        solver_data = calc_two_nums(num1, num2, oper1);  // 1.2+3.4
        if (solver_data.error == 2) {
            printf("Error: Division by zero (num1 / num2).\n");
        }
        return solver_data;
    }

    // writing an operator
    char oper2;
    oper2 = msg[i];
    i++;

    // declaring num3
    bool num3_started = false;
    bool num3_point = false;
    decimal_amount = 1;
    double num3 = 0;
    num3_started = true;

    // writing while numbers
    while (i < msg_len && i < msg_len && '0' <= msg[i] && msg[i] <= '9') {
        num3 = num3 * 10 + (msg[i] - '0');
        i++;
    }

    if (i >= msg_len)
    {
        solver_data = calc_three_nums(num1, num2, num3, oper1, oper2, bracket_pos1, bracket_pos2, bracket_pos3, bracket_pos4);
        if (solver_data.error == 2) {
            printf("Error: Division by zero.\n");
        }
        return solver_data;
    }

    if (msg[i] == ')')
    {
        bracket_pos4 = true;
        i++;

        solver_data = calc_three_nums(num1, num2, num3, oper1, oper2, bracket_pos1, bracket_pos2, bracket_pos3, bracket_pos4);
        if (solver_data.error == 2) {
            printf("Error: Division by zero.\n");
        }
        return solver_data;
        i++;
    }

    if (msg[i] == '.')
    {
        num3_point = true;
        i++;

        // continue collecting num3 after point
        while (i < msg_len)
        {
            if ('0' <= msg[i] && msg[i] <= '9')
            {
                num3 = num3 * 10 + (msg[i] - '0');
                decimal_amount *= 10;
                i++;
            }
            else if (msg[i] == ')')
            {
                bracket_pos4 = true;
                i++;
                num3 = num3 / decimal_amount;

                solver_data = calc_three_nums(num1, num2, num3, oper1, oper2, bracket_pos1, bracket_pos2, bracket_pos3, bracket_pos4);
                if (solver_data.error == 2) {
                    printf("Error: Division by zero.\n");
                }
                return solver_data;
            }
        }
        num3 = num3 / decimal_amount;  // num3 has ended
    }

    solver_data = calc_three_nums(num1, num2, num3, oper1, oper2, bracket_pos1, bracket_pos2, bracket_pos3, bracket_pos4);
    if (solver_data.error == 2) {
        printf("Error: Division by zero.\n");
    }
    return solver_data;
}


/*int main() {
    char msg[100];
    printf("Enter the task: ");
    fgets(msg, sizeof(msg), stdin);

    msg[strcspn(msg, "\n")] = '\0';
    remove_spaces(msg);
    int msg_len = strlen(msg);

    struct SolverData solver_data;
    solver_data = task_solver(msg, msg_len);

    printf("Result: %f\n", solver_data.result);
    printf("Errors: %d", solver_data.error);

    return 0;
}*/


