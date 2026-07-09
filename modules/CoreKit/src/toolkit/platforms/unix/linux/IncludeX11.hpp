/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <cstddef>
#include <stddef.h>

// Hide warning from old headers
#define register

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#undef register
#undef None
#undef Always
#undef Bool
#undef Status
#undef True
#undef False
#undef XA_ATOM
#undef XA_CARDINAL
#undef XA_STRING

namespace X11
{
    using ::Atom;
    using ::Cursor;
    using ::Display;
    using ::KeySym;
    using ::Pixmap;
    using ::Time;
    using ::Window;
    using ::XColor;
    using ::XEvent;
    using ::XSelectionEvent;
    using ::XSelectionRequestEvent;
    using ::XSetWindowAttributes;
    using ::XSizeHints;
    using ::XWindowAttributes;

    using ::XBlackPixel;
    using ::XChangeProperty;
    using ::XCloseDisplay;
    using ::XConvertSelection;
    using ::XCreateBitmapFromData;
    using ::XCreateFontCursor;
    using ::XCreatePixmapCursor;
    using ::XCreateSimpleWindow;
    using ::XCreateWindow;
    using ::XDefaultDepth;
    using ::XDefaultRootWindow;
    using ::XDefaultScreen;
    using ::XDefaultVisual;
    using ::XDefineCursor;
    using ::XDeleteProperty;
    using ::XDestroyWindow;
    using ::XDisplayHeight;
    using ::XDisplayWidth;
    using ::XDisplayWidthMM;
    using ::XFlush;
    using ::XFree;
    using ::XFreeCursor;
    using ::XFreePixmap;
    using ::XGetInputFocus;
    using ::XGetSelectionOwner;
    using ::XGetWindowAttributes;
    using ::XGetWindowProperty;
    using ::XGrabPointer;
    using ::XInitThreads;
    using ::XInternAtom;
    using ::XLookupKeysym;
    using ::XLookupString;
    using ::XMapRaised;
    using ::XMapWindow;
    using ::XMoveResizeWindow;
    using ::XMoveWindow;
    using ::XNextEvent;
    using ::XOpenDisplay;
    using ::XPending;
    using ::XQueryPointer;
    using ::XRaiseWindow;
    using ::XRootWindow;
    using ::XScreenCount;
    using ::XSendEvent;
    using ::XSetIconName;
    using ::XSetInputFocus;
    using ::XSetNormalHints;
    using ::XSetSelectionOwner;
    using ::XSetTransientForHint;
    using ::XSetWMProtocols;
    using ::XStoreName;
    using ::XTranslateCoordinates;
    using ::XUngrabPointer;
    using ::XUnmapWindow;
    using ::XWarpPointer;
}

inline constexpr X11::Atom XA_ATOM = static_cast<X11::Atom>(4);
inline constexpr X11::Atom XA_CARDINAL = static_cast<X11::Atom>(6);
inline constexpr X11::Atom XA_STRING = static_cast<X11::Atom>(31);

#endif
