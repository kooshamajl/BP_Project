#include <ncurses.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h> // For INT_MAX and INT_MIN

struct subtask
{
    char subtask_name[50];
    bool complete;
};

struct task
{
    char task_name[50];
    struct subtask SubTasks[10];
    int subtask_count;
    char Categories[10][50];
    int category_count;
    char description[400];
    struct tm deadline;
    bool complete;
};

struct task tasks[10] = {};
int task_count = 0;
int selected_task = 0;
int selected_subtask = 0;
int selected_category = 0;
bool in_subtasks_mode = false;
bool in_categories_mode = false;
char task_input[200];

void clear_line(WINDOW *win, int y)
{
    wmove(win, y, 0);
    wclrtoeol(win);
    box(win, 0, 0);
}

void delete_task(struct task tasks[], int *task_count, int selected_task)
{
    tasks[selected_task].subtask_count = 0;
    tasks[selected_task].category_count = 0;
    strcpy(tasks[selected_task].description, "");

    struct tm tm_reset = {0};
    tasks[selected_task].deadline = tm_reset;

    for (int i = selected_task; i < *task_count - 1; i++)
        tasks[i] = tasks[i + 1];

    (*task_count)--;
}

int compare_deadline(const void *a, const void *b)
{
    struct task *t1 = (struct task *)a;
    struct task *t2 = (struct task *)b;

    time_t d1 = mktime(&t1->deadline);
    time_t d2 = mktime(&t2->deadline);

    double diff = difftime(d1, d2);

    if (diff > INT_MAX)
    {
        return INT_MIN;
    }
    else if (diff < INT_MIN)
    {
        return INT_MAX;
    }

    return (int)diff;
}

int compare_alphabet(const void *a, const void *b)
{
    struct task *t1 = (struct task *)a;
    struct task *t2 = (struct task *)b;
    return strcmp(t1->task_name, t2->task_name);
}

// Function to write tasks to a file
void write_tasks_to_file(const char *filename, struct task *tasks, int task_count)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    fwrite(&task_count, sizeof(int), 1, file);

    for (int i = 0; i < task_count; i++)
    {
        fwrite(tasks + i, sizeof(struct task), 1, file);
    }

    fclose(file);
}

void read_tasks_from_file(const char *filename, struct task *tasks, int *task_count)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        return;
    }

    fread(task_count, sizeof(int), 1, file);

    for (int i = 0; i < *task_count; i++)
    {
        fread(tasks + i, sizeof(struct task), 1, file);
    }

    fclose(file);
}

int main()
{
    initscr();
    start_color();
    curs_set(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_color(COLOR_YELLOW, 700, 550, 300);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    WINDOW *tasks_win = newwin(22, 66, 1, 1);
    refresh();
    wattron(tasks_win, COLOR_PAIR(1));
    box(tasks_win, 0, 0);
    mvwprintw(tasks_win, 0, 1, "Tasks");
    wrefresh(tasks_win);

    WINDOW *SubTasks_win = newwin(10, 66, 23, 1);
    refresh();
    wattron(SubTasks_win, COLOR_PAIR(1));
    box(SubTasks_win, 0, 0);
    mvwprintw(SubTasks_win, 0, 1, "SubTasks");
    wrefresh(SubTasks_win);

    WINDOW *description_win = newwin(10, 66, 1, 67);
    refresh();
    wattron(description_win, COLOR_PAIR(1));
    box(description_win, 0, 0);
    mvwprintw(description_win, 0, 1, "Description");
    wrefresh(description_win);

    WINDOW *categories_win = newwin(8, 66, 11, 67);
    refresh();
    wattron(categories_win, COLOR_PAIR(1));
    box(categories_win, 0, 0);
    mvwprintw(categories_win, 0, 1, "Categories");
    wrefresh(categories_win);

    WINDOW *deadline_win = newwin(4, 66, 19, 67);
    refresh();
    wattron(deadline_win, COLOR_PAIR(1));
    box(deadline_win, 0, 0);
    mvwprintw(deadline_win, 0, 1, "Deadline");
    wrefresh(deadline_win);

    WINDOW *keylist_win = newwin(10, 66, 23, 67);
    refresh();
    wattron(keylist_win, COLOR_PAIR(1));
    box(keylist_win, 0, 0);
    mvwprintw(keylist_win, 0, 1, "Help");
    mvwprintw(keylist_win, 2, 1, "Use a, d, j, k key in task, subtask and category windows");
    mvwprintw(keylist_win, 4, 1, "a => Add  | d => Delete   | j or k => Selecet ");
    mvwprintw(keylist_win, 5, 1, "e => Edit | n => Deadline | l or h => Open or Close subtask_win");
    mvwprintw(keylist_win, 6, 1, "q => Quit | c => Categroy | space  => Complete task or subtasks");
    mvwprintw(keylist_win, 7, 1, "w => Save | r => Descs.   | b or s => Sort(Alphabet or deadilne)");
    wrefresh(keylist_win);

    read_tasks_from_file("tasks.dt", tasks, &task_count);

    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].complete == 1)
            mvwprintw(tasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[i].task_name);

        else
            mvwprintw(tasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[i].task_name);
    }

    wrefresh(tasks_win);

    while (1)
    {
        int ch = getch();

        if (!in_subtasks_mode && !in_categories_mode)
        {
            if (ch == 'a' && task_count < 10)
            {
                echo();
                wattron(tasks_win, A_BLINK);
                mvwprintw(tasks_win, task_count + 1, 1, "%d.[ ] ", task_count + 1);
                wrefresh(tasks_win);
                mvwgetnstr(tasks_win, task_count + 1, 7, task_input, sizeof(task_input));
                strcpy(tasks[task_count].task_name, task_input);
                task_count++;
                selected_task = task_count - 1;
                wattroff(tasks_win, A_BLINK);
                noecho();
            }

            else if (ch == 'n' && task_count > 0)
            {
                echo();
                wattron(deadline_win, A_BLINK);

                mvwscanw(deadline_win, 1, 1, "%d/%d/%d %d:%d",
                         &tasks[selected_task].deadline.tm_mday,
                         &tasks[selected_task].deadline.tm_mon,
                         &tasks[selected_task].deadline.tm_year,
                         &tasks[selected_task].deadline.tm_hour,
                         &tasks[selected_task].deadline.tm_min);

                tasks[selected_task].deadline.tm_isdst = -1;
                tasks[selected_task].deadline.tm_year -= 1900;
                tasks[selected_task].deadline.tm_mon--;

                noecho();
                wattroff(deadline_win, A_BLINK);
                wrefresh(deadline_win);
            }

            else if (ch == 'j' && selected_task < task_count - 1)
            {
                selected_task++;
                for (int i = 1; i <= 10; i++)
                    clear_line(SubTasks_win, i);

                mvwprintw(SubTasks_win, 0, 1, "SubTasks");

                for (int i = 0; i < tasks[selected_task].subtask_count; i++)
                {
                    if (tasks[selected_task].SubTasks[i].complete == 1)
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                    else
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                }

                for (int i = 0; i < 10; i++)
                    clear_line(categories_win, i);

                mvwprintw(categories_win, 0, 1, "Categories");

                wrefresh(SubTasks_win);
                wrefresh(categories_win);
            }

            else if (ch == 'k' && selected_task > 0)
            {
                selected_task--;
                for (int i = 1; i <= 10; i++)
                    clear_line(SubTasks_win, i);

                mvwprintw(SubTasks_win, 0, 1, "SubTasks");

                for (int i = 0; i < tasks[selected_task].subtask_count; i++)
                {
                    if (tasks[selected_task].SubTasks[i].complete == 1)
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                    else
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                }
                for (int i = 1; i <= 10; i++)
                    clear_line(categories_win, i);

                mvwprintw(categories_win, 0, 1, "Categories");
                wrefresh(categories_win);
                wrefresh(SubTasks_win);
            }

            else if (ch == 'd' && task_count > 0)
            {
                delete_task(tasks, &task_count, selected_task);
                if (tasks[selected_task].complete = 1)
                    tasks[selected_task].complete = 0;

                if (selected_task >= task_count && selected_task > 0)
                    selected_task = task_count - 1;

                for (int i = 0; i < 22; i++)
                    clear_line(tasks_win, i + 1);

                for (int j = 1; j <= 10; j++)
                    clear_line(description_win, j);

                for (int k = 1; k <= 10; k++)
                    clear_line(SubTasks_win, k);

                for (int h = 0; h <= 10; h++)
                    clear_line(categories_win, h);

                clear_line(deadline_win, 1);
                clear_line(deadline_win, 2);

                mvwprintw(description_win, 0, 1, "Description");
                mvwprintw(tasks_win, 0, 1, "Tasks");
                mvwprintw(SubTasks_win, 0, 1, "SubTasks");
                mvwprintw(categories_win, 0, 1, "Categories");
                mvwprintw(deadline_win, 0, 1, "Deadline");

                for (int i = 0; i < task_count; i++)
                {
                    if (tasks[i].complete == 1)
                        mvwprintw(tasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[i].task_name);
                    else
                        mvwprintw(tasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[i].task_name);
                }

                wrefresh(tasks_win);
                wrefresh(description_win);
                wrefresh(SubTasks_win);
                wrefresh(deadline_win);
                wrefresh(categories_win);
            }

            else if (ch == ' ')
                tasks[selected_task].complete = !tasks[selected_task].complete;

            else if (ch == 'e' && task_count > 0)
            {
                echo();
                clear_line(tasks_win, selected_task + 1);
                mvwprintw(tasks_win, 0, 1, "Tasks");
                wrefresh(tasks_win);
                mvwprintw(tasks_win, selected_task + 1, 1, "%d.[ ]", selected_task + 1);
                mvwgetnstr(tasks_win, selected_task + 1, 7, tasks[selected_task].task_name, sizeof(tasks[selected_task].task_name));
                wattron(tasks_win, A_STANDOUT);
                wrefresh(tasks_win);
                wattroff(tasks_win, A_STANDOUT);
                noecho();
            }

            else if (ch == 'r' && task_count > 0)
            {
                echo();
                wattron(description_win, A_BLINK);
                mvwgetnstr(description_win, 1, 1, task_input, sizeof(task_input));
                strcpy(tasks[selected_task].description, task_input);
                wrefresh(description_win);
                wattroff(description_win, A_BLINK);
                noecho();
            }

            else if (ch == 'l' && task_count > 0)
            {
                in_subtasks_mode = true;
                selected_subtask = 0;
            }

            else if (ch == 'c')
            {
                in_categories_mode = true;
                selected_category = 0;
            }

            else if (ch == 'w')
            {
                write_tasks_to_file("tasks.dt", tasks, task_count);
            }

            else if (ch == 'q')
            {
                break;
            }

            else if (ch == 's')
            {
                qsort(tasks, task_count, sizeof(struct task), compare_deadline);
            }

            else if (ch == 'b')
            {
                qsort(tasks, task_count, sizeof(struct task), compare_alphabet);
            }
        }

        else if (in_subtasks_mode)
        {
            if (ch == 'j' && selected_subtask < tasks[selected_task].subtask_count - 1)
                selected_subtask++;

            else if (ch == 'k' && selected_subtask > 0)
                selected_subtask--;

            else if (ch == ' ')
            {
                tasks[selected_task].SubTasks[selected_subtask].complete = !tasks[selected_task].SubTasks[selected_subtask].complete;
                wrefresh(SubTasks_win);
            }

            else if (ch == 'a' && tasks[selected_task].subtask_count < 10)
            {
                echo();
                wattron(SubTasks_win, A_BLINK);
                mvwprintw(SubTasks_win, tasks[selected_task].subtask_count + 1, 1, "%d.[ ] ", tasks[selected_task].subtask_count + 1);
                wrefresh(SubTasks_win);
                mvwgetnstr(SubTasks_win, tasks[selected_task].subtask_count + 1, 6, task_input, sizeof(task_input));
                strcpy(tasks[selected_task].SubTasks[tasks[selected_task].subtask_count].subtask_name, task_input);
                tasks[selected_task].SubTasks[tasks[selected_task].subtask_count].complete = 0;
                tasks[selected_task].subtask_count++;
                selected_subtask = tasks[selected_task].subtask_count - 1;
                wattroff(SubTasks_win, A_BLINK);
                noecho();
                wrefresh(SubTasks_win);
            }

            else if (ch == 'd' && tasks[selected_task].subtask_count > 0)
            {
                for (int i = selected_subtask; i < tasks[selected_task].subtask_count - 1; i++)
                    tasks[selected_task].SubTasks[i] = tasks[selected_task].SubTasks[i + 1];

                tasks[selected_task].subtask_count--;

                if (tasks[selected_task].subtask_count > 0 && selected_subtask >= tasks[selected_task].subtask_count)
                    selected_subtask = tasks[selected_task].subtask_count - 1;

                for (int i = 1; i <= 10; i++)
                    clear_line(SubTasks_win, i);

                mvwprintw(SubTasks_win, 0, 1, "SubTasks");
                wrefresh(SubTasks_win);

                for (int i = 0; i < tasks[selected_task].subtask_count; i++)
                {
                    if (i == selected_subtask)
                        wattron(SubTasks_win, A_STANDOUT);

                    if (tasks[selected_task].SubTasks[i].complete == 1)
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                    else
                        mvwprintw(SubTasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);

                    if (i == selected_subtask)
                        wattroff(SubTasks_win, A_STANDOUT);
                }
                wrefresh(SubTasks_win);
            }

            else if (ch == 'h')
                in_subtasks_mode = false;

            else if (ch == 'c')
                in_categories_mode = true;
        }

        else if (in_categories_mode)
        {
            if (ch == 'a' && tasks[selected_task].category_count < 10)
            {
                echo();
                wattron(categories_win, A_BLINK);
                mvwprintw(categories_win, tasks[selected_task].category_count + 1, 1, "%d. ", tasks[selected_task].category_count + 1);
                wrefresh(categories_win);

                mvwgetnstr(categories_win, tasks[selected_task].category_count + 1, 4, task_input, sizeof(task_input));
                strcpy(tasks[selected_task].Categories[tasks[selected_task].category_count], task_input);
                tasks[selected_task].category_count++;
                selected_category = tasks[selected_task].category_count - 1;
                wattroff(categories_win, A_BLINK);
                noecho();
                wrefresh(categories_win);
            }

            else if (ch == 'd' && tasks[selected_task].category_count > 0)
            {
                for (int i = selected_category; i < tasks[selected_task].category_count - 1; i++)
                    strcpy(tasks[selected_task].Categories[i], tasks[selected_task].Categories[i + 1]);

                tasks[selected_task].category_count--;

                if (tasks[selected_task].category_count > 0 && selected_category >= tasks[selected_task].category_count)
                    selected_category = tasks[selected_task].category_count - 1;

                for (int i = 1; i <= 10; i++)
                    clear_line(categories_win, i);

                mvwprintw(categories_win, 0, 1, "Categories");

                for (int i = 0; i < tasks[selected_task].category_count; i++)
                {
                    if (i == selected_category)
                        wattron(categories_win, A_STANDOUT);

                    mvwprintw(categories_win, i + 1, 1, "%d. %s", i + 1, tasks[selected_task].Categories[i]);

                    if (i == selected_category)
                        wattroff(categories_win, A_STANDOUT);
                }
                wrefresh(categories_win);
            }
            else if (ch == 'j' && selected_category < tasks[selected_task].category_count - 1)
                selected_category++;

            else if (ch == 'k' && selected_category > 0)
                selected_category--;

            else if (ch == 'c')
                in_categories_mode = false;
        }

        for (int i = 0; i < 10; i++)
        {
            clear_line(description_win, i + 1);
        }

        mvwprintw(description_win, 0, 1, "Description");
        wrefresh(description_win);

        for (int j = 0; j < 10; j++)
        {
            clear_line(categories_win, j + 1);
        }

        mvwprintw(categories_win, 0, 1, "Categories");

        for (int i = 0; i < task_count; i++)
        {

            clear_line(tasks_win, i + 1);

            if (i == selected_task)
            {
                char time_str[80];
                clear_line(deadline_win, 1);
                clear_line(deadline_win, 2);
                mvwprintw(deadline_win, 0, 1, "Deadline");
                wattron(tasks_win, A_STANDOUT);
                struct tm tm_reset = {0};
                time_t reset = mktime(&tm_reset);
                time_t deadline = mktime(&tasks[selected_task].deadline);

                if (difftime(reset, deadline))
                {
                    strftime(time_str, sizeof(time_str), "%a %b %d %R %Y", &tasks[selected_task].deadline);
                    mvwprintw(deadline_win, 1, 1, "%s", time_str);

                    time_t current = time(NULL);
                    double diff_seconds = difftime(deadline, current);

                    int days = diff_seconds / 86400;
                    int hours = ((int)diff_seconds / 3600) % 24;
                    int minutes = ((int)diff_seconds / 60) % 60;
                    mvwprintw(deadline_win, 2, 1, "Until Deadline: %d days | %d hrs | %d mins", days, hours, minutes);
                }
            }

            wrefresh(deadline_win);

            if (i == selected_task && selected_task >= 0)
            {
                mvwprintw(description_win, 1, 1, "%s", tasks[i].description);
                wrefresh(description_win);
            }

            if (tasks[i].complete == 1)
                mvwprintw(tasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[i].task_name);

            else
                mvwprintw(tasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[i].task_name);

            if (i == selected_task)
                wattroff(tasks_win, A_STANDOUT);

            if (i == selected_task && selected_task >= 0)
            {
                for (int j = 0; j < tasks[i].category_count; j++)
                {
                    if (j == selected_category)
                        wattron(categories_win, A_STANDOUT);

                    mvwprintw(categories_win, j + 1, 1, "%d. %s", j + 1, tasks[i].Categories[j]);

                    if (j == selected_category)
                        wattroff(categories_win, A_STANDOUT);
                }
                wrefresh(categories_win);
            }
        }

        if (in_subtasks_mode)
        {
            for (int i = 0; i < tasks[selected_task].subtask_count; i++)
            {
                if (i == selected_subtask)
                    wattron(SubTasks_win, A_STANDOUT);

                if (tasks[selected_task].SubTasks[i].complete == 1)
                    mvwprintw(SubTasks_win, i + 1, 1, "%d.[X] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);

                else
                    mvwprintw(SubTasks_win, i + 1, 1, "%d.[ ] %s", i + 1, tasks[selected_task].SubTasks[i].subtask_name);
                if (i == selected_subtask)
                    wattroff(SubTasks_win, A_STANDOUT);
            }
            wrefresh(SubTasks_win);
        }

        wrefresh(categories_win);
        wrefresh(description_win);
        wrefresh(tasks_win);
        wrefresh(deadline_win);
    }

    endwin();
    return 0;
}
