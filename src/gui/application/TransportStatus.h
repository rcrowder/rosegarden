// -*- c-basic-offset: 4 -*-
/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
    See the AUTHORS file for more details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#ifndef _ROSEGARDEN_TRANSPORT_STATUS_H_
#define _ROSEGARDEN_TRANSPORT_STATUS_H_

typedef enum
{
     STOPPED,
     PLAYING,
     RECORDING,
     STOPPING,
     STARTING_TO_PLAY,
     STARTING_TO_RECORD,
     RECORDING_ARMED,                   // gui only state
     QUIT
} TransportStatus;

#endif // _ROSEGARDEN_TRANSPORT_STATUS_H_
