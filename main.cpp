#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

using namespace std;
static const int MAX_LEN = 255;

string getDirectory()
{

    char buffer[MAX_LEN];
    getcwd(buffer, MAX_LEN);
    string path(buffer);
    return path;
}

vector<string> *getFiles(string path)
{
    static vector<string> *files = new vector<string>();
    DIR *dir;
    struct dirent *dPrt;
    char name[MAX_LEN];
    if ((dir = opendir(path.c_str())) == NULL)
    {
        return files;
    }
    while ((dPrt = readdir(dir)) != NULL)
    {
        if (strcmp(dPrt->d_name, ".") == 0 || strcmp(dPrt->d_name, "..") == 0)
        {
            continue;
        }
        else if (dPrt->d_type == 8 || dPrt->d_type == 10 || dPrt->d_type == 4)
        {
            strcpy(name, dPrt->d_name);
            files->push_back(string(name));
        }
    }
    closedir(dir);
    return files;
}

class Drawable
{
public:
    virtual void draw() = 0;
    virtual WINDOW *getWin() const = 0;
    virtual int getX() const { return 0; };
    virtual int getY() const { return 0; };
    virtual int getW() const { return 0; };
    virtual int getH() const { return 0; };
};

class Header : Drawable
{
private:
    string title;
    int w;
    int h;
    int x;
    int y;
    int tx;
    int ty;
    WINDOW *win;

public:
    Header(string title, int x, int y, int w, int h) : title(title), x(x), y(y), w(w), h(h)
    {
        tx = x + 1;
        ty = (int)h / 2;
        win = newwin(h, w, y, x);
    };
    ~Header()
    {
        delwin(win);
    }
    string getTitle() const { return title; };
    void setTitle(string nTitle) { title = nTitle; };
    int getX() const { return x; };
    int getY() const { return y; };
    int getW() const { return w; };
    int getH() const { return h; };
    int getTX() const { return tx; };
    int getTY() const { return ty; };
    WINDOW *getWin() const { return win; };
    void draw()
    {
        box(win, ACS_VLINE, ACS_HLINE);
        wmove(win, ty, tx);
        wprintw(win, "MowFinder:%s", title.c_str());
        wrefresh(win);
    };
};

class MenuItem
{
private:
public:
    MenuItem(string name) : name(name)
    {
        this->selected = false;
    }
    string name;
    bool toggleSelect()
    {
        return selected = !selected;
    }
    bool selected;
};

class Menu : Drawable
{
private:
    WINDOW *win;
    int w;
    int h;
    int x;
    int y;
    int lx;
    int ly;
    int activeIndex;
    int page;
    int maxPage;
    int pageSize;
    vector<MenuItem *> *list;

public:
    Menu(vector<MenuItem *> *list, int x, int y, int w, int h) : list(list), x(x), y(y), w(w), h(h)
    {
        activeIndex = 0;
        page = 0;
        pageSize = h - 2;
        maxPage = (int)(list->size() / pageSize);
        menuList = new vector<MenuItem *>();
        lx = 1;
        ly = 1;
        win = newwin(h, w, y, x);
    }
    ~Menu()
    {
        delwin(win);
        delete list;
    }
    void checkClear(const int lastPage, const int currentPage)
    {
        if (lastPage != currentPage)
        {
            wclear(win);
        }
    }
    void updatePage()
    {
        int last = page;
        page = activeIndex / pageSize;
        checkClear(last, page);
    }
    void draw()
    {
        box(win, ACS_VLINE, ACS_HLINE);
        const int listSize = list->size();
        int start = page * pageSize;
        int len = start + pageSize;
        if (page >= maxPage)
        {
            len = listSize > len ? len : listSize;
        }
        for (int i = start; i < len; i++)
        {
            wmove(win, ly + (i % pageSize), lx);
            auto item = list->at(i);
            if (activeIndex == i)
            {
                auto cp = COLOR_PAIR(1);
                wattron(win, cp);
                wprintw(win, " %s ", item->name.c_str());
                wattroff(win, cp);
            }
            else
            {
                wprintw(win, " %s ", item->name.c_str());
            }
        }

        wrefresh(win);
    };
    vector<MenuItem *> *menuList;
    WINDOW *getWin() const { return win; };
    void nextItem()
    {
        if (activeIndex + 1 < list->size())
        {
            activeIndex += 1;
        }
        else
        {
            activeIndex = 0;
        }
        updatePage();
    };
    void lastItem()
    {
        if (activeIndex - 1 > 0)
        {
            activeIndex -= 1;
        }
        else
        {
            activeIndex = list->size() - 1;
        }
        updatePage();
    };
    bool jumpItem(const int i)
    {
        if (i < 0 || i > list->size() - 1)
        {
            return false;
        }
        activeIndex = i;
        updatePage();
        return true;
    };
};

class Context
{
private:
    Header *header;
    Menu *menu;

public:
    Context(
        string headerTitle, vector<MenuItem *> *list)
    {
        header = new Header(headerTitle, 0, 0, COLS, 3);
        menu = new Menu(
            list, 0, header->getH(), (int)COLS / 10 * 4, LINES - header->getH() - header->getY() - 1);
    };
    ~Context()
    {
        delete header;
        delete menu;
    }
    void update()
    {
        header->draw();
        menu->draw();
        refresh();
    }
    Header *getHeader() const { return header; }
    Menu *getMenu() const { return menu; }
};

int main(int, char **)
{
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE); //定义颜色组合
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, 1);
    auto currentPath = getDirectory();
    auto files = getFiles(currentPath);
    vector<MenuItem *> *menuList = new vector<MenuItem *>();
    auto context = new Context(
        currentPath,
        menuList);
    for (int i = 0; i < files->size(); i++)
    {
        auto file = (*files)[i];
        menuList->push_back(new MenuItem(file));
    }

    int readKey;
    context->update();
    do
    {
        context->update();
        readKey = getch();
        if (readKey == ERR || readKey == 'q')
        {
            break;
        }
        else if (readKey == KEY_UP)
        {
            context->getMenu()->lastItem();
        }
        else if (readKey == KEY_DOWN)
        {
            context->getMenu()->nextItem();
        }
    } while (true);
    refresh();
    endwin();
    delete files;
    return 0;
}
