/***************************************************************************
 *   cthost.cpp                                                            *
 *   --------------------------------------------------------------------  *
 *   CT Host Implementation                                                *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 ***************************************************************************/

#include "ctdebug.h"
#include "cthost.h"
#include <unistd.h>
#include <stdio.h>
#include <fstream.h>

CTHost::CTHost()
{
  CTDEBUG("CTHost constructor");

  // If it is the root user
  if (getuid() == 0)
  {
    // Create the system cron table.
    createCTCron(true);

    // Get a user list out of the password file
    ifstream inputStream("/etc/passwd");

    const int MAX = 1024;
    char buffer[MAX];
    string line;
    string login;

    // Create each user's cron table.
    while (inputStream)
    {
      inputStream.getline(buffer, MAX);
      line = buffer;
      login = line.substr(0,line.find(":"));
      if (login != "")
        createCTCron(false, login);
    }
  }
  else
  // Non-root user, so just create user's cron table.
  {
    createCTCron();
  }
}

CTHost::~CTHost()
{
  CTDEBUG("CTHost destructory");
  for (CTCronIterator i = cron.begin(); i != cron.end(); i++)
    delete *i;
}

void CTHost::apply()
{
  CTDEBUG("CTHost apply");
  for (CTCronIterator i = cron.begin(); i != cron.end(); i++)
    (*i)->apply();
}

void CTHost::cancel()
{
  CTDEBUG("CTHost cancel");
  for (CTCronIterator i = cron.begin(); i != cron.end(); i++)
    (*i)->cancel();
}

bool CTHost::dirty()
{
  CTDEBUG("CTHost dirty");
  bool isDirty(false);

  for (CTCronIterator i = cron.begin(); i != cron.end(); i++)
    if ((*i)->dirty()) isDirty = true;

  return isDirty;
}

CTCron* CTHost::createCTCron(bool _syscron, string _login)
{
  CTDEBUG("CTHost createCTCron");
  CTCron *p = new CTCron(_syscron, _login);
  cron.push_back(p);
  return p;
}

bool CTHost::root() const
{
  CTDEBUG("CTHost root");
  return (!getuid());
}