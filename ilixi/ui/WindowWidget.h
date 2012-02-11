/*
 Copyright 2010, 2011 Tarik Sekmen.

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

#ifndef ILIXI_WINDOWWIDGET_H_
#define ILIXI_WINDOWWIDGET_H_

#include "core/Window.h"
#include "ui/Frame.h"
#include <vector>
#include "types/Rectangle.h"
#include <semaphore.h>

namespace ilixi
{
  //! Base class for windowed widgets.
  class WindowWidget : public Window, public Frame
  {
  public:
    /*!
     * Constructor.
     */
    WindowWidget(Widget* parent = 0);

    /*!
     * Destructor.
     */
    virtual
    ~WindowWidget();

    /*!
     * Returns and interface to the focus manager.
     */
    EventManager* const
    windowEventManager() const;

    /*!
     * This method executes update().
     */
    virtual void
    doLayout();

    /*!
     * Paints inside given rectangle.
     *
     * Actual painting takes place once a union rectangle is
     * formed by updateWindow or repaint methods.
     */
    virtual void
    paint(const Rectangle& rect);

    /*!
     * Immediately repaints inside given rectangle.
     */
    virtual void
    repaint(const Rectangle& rect);

  protected:
    /*!
     * Create and initialise DirectFB window using preferred size.
     */
    void
    initWindow();

    /*!
     * Sets visibility to false and releases window and surface resources of all children.
     */
    void
    closeWindow();

    /*!
     * This method executes before sending key down events to
     * focused widget.
     *
     * @return true if event is consumed.
     */
    virtual bool
    keyListener(DFBInputDeviceKeySymbol key);

    /*!
     * Handles incoming input events if window has focus.
     * Behaviour is dependant on incoming event type. Key events are sent to focused widget.
     * Pointer events are consumed by the widget which contains the event coordinates. If two or more
     * widgets contain the event coordinates, event is sent to the widget at the top.
     *
     * @param event parsed by DirectFB interfaces.
     * @return True if event is consumed.
     */
    bool
    handleWindowEvent(const DFBWindowEvent& event);

    /*!
     * This signal is emitted when window is aborted/closed.
     */
    sigc::signal<void> sigAbort;

  private:
    //! Stores window's dirty regions and a region for update.
    struct
    {
      pthread_mutex_t _listLock;
      sem_t _updateReady;
      sem_t _paintReady;
      Rectangle _updateRegion;
      std::vector<Rectangle> _updateQueue;
    } _updates;

    /*!
     * Creates a united rectangle from a list of dirty regions and
     * performs a paint operation on this united rectangle.
     */
    virtual void
    updateWindow();

  };

}

#endif /* ILIXI_WINDOWWIDGET_H_ */
