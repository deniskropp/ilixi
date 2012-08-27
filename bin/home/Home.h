/*
 Copyright 2010-2012 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 This file is part of ilixi.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with ilixi.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HOME_H_
#define HOME_H_

#include <ui/Application.h>
#include <types/Font.h>
#include <core/DaleDFB.h>

namespace ilixi
{

class Home : public Application
{
    struct AppData
    {
        char name[128];
        char icon[256];
    };

    typedef std::vector<AppData> AppDataVector;

public:
    Home(int argc, char* argv[]);

    virtual
    ~Home();

    void
    runApp(const char* name);

protected:
    void
    initButtons(const AppDataVector& dataVector);

    void
    addButton(const char* name, const char* icon);

private:
    IComaComponent* _compositor;
    Font* _font;

    void
    updateHomeGeometry();

    friend void
    receiveAppList(void* ctx, void* arg);
};

}

#endif /* HOME_H_ */
