/***************************************************************************
 *   kttask.h                                                              *
 *   --------------------------------------------------------------------  *
 *   KT task editor window header                                          *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 ***************************************************************************/

#ifndef KTTASK_H
#define KTTASK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qdialog.h>
class QLabel;
class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QPushButton;
class KAccel;

class CTTask;

/**
  * Task editor window.
  */
class KTTask : public QDialog
{
  Q_OBJECT

public:

/**
  * Initialize from CTTask.
  */
  KTTask(CTTask* _cttask = 0);

/**
  * Destroy.
  */
  ~KTTask();

private slots:

/**
  * Browse for command file.
  */
  void slotBrowse();

/**
  * Command has been changed.  Refresh icon.
  */
  void slotCommandChanged();

/**
  * Daily checkbox has been changed.
  */
  void slotDailyChanged();

/**
  * Apply changes and close.
  */
  void slotOK();

/**
  * Cancel and close.
  */
  void slotCancel();

private:

/**
  * Task.
  */
  CTTask*         cttask;

/**
  * Key accelerator.
  */

  KAccel*         key_accel;

  // Widgets.

  QLabel*         labUser;
  QLineEdit*      leUser;

  QLabel*         labIcon;

  QLabel*         labComment;
  QLineEdit*      leComment;

  QLabel*         labCommand;
  QLineEdit*      leCommand;
  QPushButton*    pbBrowse;

  QCheckBox*      chkEnabled;

  QButtonGroup*   bgMonth;
  QCheckBox*      cbMonth[13];

  QButtonGroup*   bgDayOfMonth;
  QPushButton*    pbDayOfMonth[32];

  QButtonGroup*   bgDayOfWeek;
  QCheckBox*      cbDayOfWeek[8];

  QButtonGroup*   bgEveryDay;
  QCheckBox*      cbEveryDay;

  QButtonGroup*   bgHour;
  QLabel*         labAM;
  QLabel*         labPM;
  QPushButton*    pbHour[25];

  QButtonGroup*   bgMinute;
  QPushButton*    pbMinute[61];

  QPushButton*    pbOk;
  QPushButton*    pbCancel;

};

#endif // KTTASK_H