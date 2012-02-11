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

#include "WindowWidget.h"
#include "core/Logger.h"

using namespace ilixi;

WindowWidget::WindowWidget(Widget* parent) :
  Window(), Frame(parent)
{
  setNeighbours(this, this, this, this);
  pthread_mutex_init(&_updates._listLock, NULL);
  sem_init(&_updates._updateReady, 0, 0);
  sem_init(&_updates._paintReady, 0, 1);

  _surfaceDesc = WindowDescription;
  setRootWindow(this);
  setMargins(5, 5, 5, 5);
}

WindowWidget::~WindowWidget()
{
  pthread_mutex_destroy(&_updates._listLock);
  sem_destroy(&_updates._updateReady);
  sem_destroy(&_updates._paintReady);
}

EventManager* const
WindowWidget::windowEventManager() const
{
  return _eventManager;
}

void
WindowWidget::doLayout()
{
  update();
}

void
WindowWidget::paint(const Rectangle& rect)
{
  if (visible())
    {
      int ready;
      sem_getvalue(&_updates._updateReady, &ready);
      if (ready)
        {
          sem_wait(&_updates._updateReady);
          updateSurface();
          Rectangle intersect = _frameGeometry.intersected(
              _updates._updateRegion);

          //          DFBRegion r = intersect.dfbRegion();
          //          _window->BeginUpdates(_window, &r);

          if (intersect.isValid())
            {
              if (_backgroundFilled)
                {
                  surface()->clear(intersect);
                  compose();
                }
              else
                surface()->clear(intersect);

              paintChildren(intersect);
              surface()->flip(intersect);
            }
          sem_post(&_updates._paintReady);
        }
      else
        {
          pthread_mutex_lock(&_updates._listLock);
          _updates._updateQueue.push_back(rect);
          pthread_mutex_unlock(&_updates._listLock);
        }
    }
}

void
WindowWidget::repaint(const Rectangle& rect)
{
  if (visible())
    {
      sem_wait(&_updates._paintReady);
      _updates._updateRegion = rect;
      sem_post(&_updates._updateReady);
      paint(_updates._updateRegion);
    }
}

void
WindowWidget::initWindow()
{
  if (_window)
    return;

  if (initDFBWindow(preferredSize()))
    {
      setSize(getWindowSize());
      setRootWindow(this);
      paint(Rectangle(0, 0, width(), height()));
    }
}

void
WindowWidget::closeWindow()
{
  setVisible(false);
  hideWindow();
  invalidateSurface();
  releaseDFBWindow();
}

bool
WindowWidget::keyListener(DFBInputDeviceKeySymbol key)
{
  return false;
}

bool
WindowWidget::handleWindowEvent(const DFBWindowEvent& event)
{
  // ignore events outside this window...
  if (getWindowID() != event.window_id)
    return false;

  // handle all other events...
  Widget* target = this;
  if (_eventManager->grabbedWidget())
    target = _eventManager->grabbedWidget();

  switch (event.type)
    {
  case DWET_CLOSE: // handle Close, can be signalled by viewport manager.
    sigAbort();
    return true;

  case DWET_LEAVE: // handle Leave, can be signalled if pointer moves outside window.
    _eventManager->setExposedWidget(NULL,
        PointerEvent(PointerMotion, event.x, event.y));
    _eventManager->setGrabbedWidget(NULL,
        PointerEvent(PointerMotion, event.x, event.y));
    return true;

  case DWET_BUTTONUP:
    _eventManager->setGrabbedWidget(
        NULL,
        PointerEvent(PointerButtonUp, event.x, event.y, 0,
            (PointerButton) event.button, (PointerButtonMask) event.buttons));
    return target->consumePointerEvent(
        PointerEvent(PointerButtonUp, event.x, event.y, 0,
            (PointerButton) event.button, (PointerButtonMask) event.buttons));

  case DWET_BUTTONDOWN:
    return target->consumePointerEvent(
        PointerEvent(PointerButtonDown, event.x, event.y, 0,
            (PointerButton) event.button, (PointerButtonMask) event.buttons));

  case DWET_MOTION:
    return target->consumePointerEvent(
        PointerEvent(PointerMotion, event.x, event.y, event.step,
            (PointerButton) event.button, (PointerButtonMask) event.buttons));

  case DWET_WHEEL:
    return target->consumePointerEvent(
        PointerEvent(PointerWheel, event.x, event.y, event.step,
            (PointerButton) event.button, (PointerButtonMask) event.buttons));

  case DWET_KEYUP:
    if (_eventManager->focusedWidget())
      return _eventManager->focusedWidget()->consumeKeyEvent(
          KeyEvent(KeyUpEvent, event.key_symbol, event.modifiers, event.locks));
    return false;

  case DWET_KEYDOWN:
    switch (event.key_symbol)
      {

    // TODO Remove ESCAPE and sigAbort() ?
    case DIKS_ESCAPE:
      sigAbort();
      return true;

    case DIKS_BACK:
      sigAbort();
      return true;

//    case DIKS_CURSOR_LEFT:
//      if (target == this)
//        {
//          _eventManager->selectNeighbour(Left);
//          return true;
//        }
//      break;
//
//    case DIKS_CURSOR_RIGHT:
//      if (target == this)
//        {
//          _eventManager->selectNeighbour(Right);
//          return true;
//        }
//      break;
//
//    case DIKS_CURSOR_UP:
//      if (target == this)
//        {
//          _eventManager->selectNeighbour(Up);
//          return true;
//        }
//      break;
//
//    case DIKS_CURSOR_DOWN:
//      if (target == this)
//        {
//          _eventManager->selectNeighbour(Down);
//          return true;
//        }
//      break;

    case DIKS_OK:
    case DIKS_RETURN:
      if (_eventManager->focusedWidget())
        {
          if (_eventManager->focusedWidget() != target)
            _rootWindow->windowEventManager()->setGrabbedWidget(
                _eventManager->focusedWidget());
          else
            _rootWindow->windowEventManager()->setGrabbedWidget(NULL);
          return true;
        }
      break;

    case DIKS_TAB: // handle TAB release.
      if (event.modifiers == DIMM_SHIFT)
        _eventManager->selectPrevious();
      else
        _eventManager->selectNext();
      return true;

      } // end switch

    if (keyListener(event.key_symbol))
      return true;

    else if (_eventManager->grabbedWidget())
      return _eventManager->grabbedWidget()->consumeKeyEvent(
          KeyEvent(KeyDownEvent, event.key_symbol, event.modifiers, event.locks));

    else if (_eventManager->focusedWidget())
      return _eventManager->focusedWidget()->consumeKeyEvent(
          KeyEvent(KeyDownEvent, event.key_symbol, event.modifiers, event.locks));
    else
      return false;

  default:
    return false;
    }
}

void
WindowWidget::updateWindow()
{
  if (!_window)
    return;

  pthread_mutex_lock(&_updates._listLock);
  int size = _updates._updateQueue.size();
  if (size == 0)
    {
      pthread_mutex_unlock(&_updates._listLock);
      return;
    }

  Rectangle updateTemp = _updates._updateQueue[0];
  if (size > 1)
    for (int i = 1; i < size; ++i)
      updateTemp = updateTemp.united(_updates._updateQueue[i]);

  _updates._updateQueue.clear();

  if (!updateTemp.isNull())
    {
      sem_wait(&_updates._paintReady);
      _updates._updateRegion = updateTemp;
      sem_post(&_updates._updateReady);
      paint(_updates._updateRegion);
    }
  pthread_mutex_unlock(&_updates._listLock);
}
