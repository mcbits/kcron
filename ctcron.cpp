/***************************************************************************
 *   ctcron.cpp                                                            *
 *   --------------------------------------------------------------------  *
 *   CT Cron Implementation                                                *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 ***************************************************************************/

#include "ctdebug.h"
#include "cti18n.h"
#include "ctcron.h"
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <fstream.h>

CTCron::CTCron(bool _syscron, string _login) :
  syscron(_syscron)
{
  CTDEBUG("CTCron constructor");

  int uid(getuid());

  char ofile[20] = "/tmp/crontab.XXXXXX";
  sprintf (ofile+13,"%d",getpid());

  tmpFileName = string(ofile);

  string readCommand;
  passwd *pwd;

  if (uid == 0)
  // root, so provide requested crontab
  {
    if (syscron)
    {
      readCommand  = "cat /etc/crontab > " + tmpFileName;
      writeCommand = "cat " + tmpFileName + " > /etc/crontab";
      login = cti18n("(System Crontab)");
      name = "";
    }
    else
    {
      readCommand  = "crontab -u " + _login + " -l > " + tmpFileName;
      writeCommand = "crontab -u " + _login + " " + tmpFileName;
      pwd = getpwnam(_login.c_str());
      if (pwd == 0)
        throw CTExceptionIO();
      login = pwd->pw_name;
      name = pwd->pw_gecos;
    }
  }
  else
  // regular user, so provide user's own crontab
  {
    readCommand  = "crontab -l > " + tmpFileName;
    writeCommand = "crontab "      + tmpFileName;
    pwd  = getpwuid(uid);
      if (pwd == 0)
        throw CTExceptionIO();
    login = pwd->pw_name;
    name = pwd->pw_gecos;
  }

  if (name == "")
    name = login;

  // Don't throw exception, if can't be read, it means the user
  // doesn't have a crontab.
  if (system(readCommand.c_str()) == 0)
  {
    ifstream cronfile(tmpFileName.c_str());
    cronfile >> *this;
  }

  if (unlink(tmpFileName.c_str()) != 0)
    throw CTExceptionIO();

  initialTaskCount      = task.size();
  initialVariableCount  = variable.size();
}

CTCron::CTCron(const CTCron& source)
{
  CTDEBUG("CTCron copy constructor");

  throw CTException();
}

void CTCron::operator = (const CTCron& source)
{
  CTDEBUG("CTCron assignment operator");

  if (source.syscron)
    throw CTException();

  for (CTVariableIterator i = (CTVariableIterator)source.variable.begin();
    i != source.variable.end(); i++)
  {
    CTVariable* tmp = new CTVariable(**i);
    variable.push_back(tmp);
  }

  for (CTTaskIterator i = (CTTaskIterator)source.task.begin();
    i != source.task.end(); i++)
  {
    CTTask* tmp = new CTTask(**i);
    task.push_back(tmp);
  }
}

istream& operator >> (istream& inputStream, CTCron& cron)
{
  CTDEBUG("CTCron insertion operator");

  const int MAX = 1024;
  char buffer[MAX];
  string line("");
  string comment("");

  while (inputStream)
  {
    inputStream.getline(buffer, MAX);
    line = buffer;

    // search for comments "#" but not disabled tasks "#\"
    if ((line.find("#") == 0) && (line.find("\\") != 1))
    {
      // remove leading pound sign
      line = line.substr(1,line.length()-1);

      // remove leading whitespace
      while (line.find_first_of(" \t") == 0)
        line = line.substr(1,line.length()-1);

      comment = line;
    }
    else
    {
      // either a task or a variable
      int firstWhiteSpace(line.find_first_of(" \t"));
      int firstEquals(line.find("="));

      // if there is an equals sign and either there is no
      // whitespace or the first whitespace is after the equals
      // sign, it must be a variable
      if ((firstEquals > 0) && ((firstWhiteSpace == -1) ||
        firstWhiteSpace > firstEquals))
      {
        // create variable
        CTVariable *tmp = new CTVariable(line, comment);
        cron.variable.push_back(tmp);
        comment = "";
      }
      else
      // must be a task, either enabled or disabled
      {
        if (firstWhiteSpace > 0)
        {
          CTTask *tmp = new CTTask(line, comment, cron.syscron);
          cron.task.push_back(tmp);
          comment = "";
        }
      }
    }
  }
  return inputStream;
}

ostream& operator << (ostream& outputStream, const CTCron& cron)
{
  CTDEBUG("CTCron extraction operator");

  for (CTVariableIterator i = (CTVariableIterator)cron.variable.begin();
    i != cron.variable.end(); i++)
    outputStream << **i;

  for (CTTaskIterator i = (CTTaskIterator)cron.task.begin();
    i != cron.task.end(); i++)
    outputStream << **i;

  return outputStream;
}

CTCron::~CTCron()
{
  CTDEBUG("CTCron destructor");

  for (CTTaskIterator i = task.begin(); i != task.end(); i++)
    delete *i;
  for (CTVariableIterator i = variable.begin(); i != variable.end(); i++)
    delete *i;
}

void CTCron::apply()
{
  CTDEBUG("CTCron.apply()");

  // write to temp file
  ofstream cronfile(tmpFileName.c_str());
  cronfile << *this << flush;

  // install temp file into crontab
  if (system(writeCommand.c_str()) != 0)
    throw CTExceptionIO();

  // remove the temp file
  if(unlink(tmpFileName.c_str()) != 0)
    throw CTExceptionIO();

  // mark as applied
  for (CTTaskIterator i = task.begin(); i != task.end(); i++)
    (*i)->apply();

  for (CTVariableIterator i = variable.begin(); i != variable.end(); i++)
    (*i)->apply();
}

void CTCron::cancel()
{
  CTDEBUG("CTCron.cancel()");

  for (CTTaskIterator i = task.begin(); i != task.end(); i++)
    (*i)->cancel();

  for (CTVariableIterator i = variable.begin(); i != variable.end(); i++)
    (*i)->cancel();
}

bool CTCron::dirty()
{
  CTDEBUG("CTCron.dirty()");

  bool isDirty(false);

  if (initialTaskCount != task.size()) isDirty = true;

  if (initialVariableCount != variable.size()) isDirty = true;

  for (CTTaskIterator i = task.begin(); i != task.end(); i++)
    if ((*i)->dirty()) isDirty = true;

  for (CTVariableIterator i = variable.begin(); i != variable.end(); i++)
    if ((*i)->dirty()) isDirty = true;

  return isDirty;
}

string CTCron::path() const
{
  CTDEBUG("CTCron.path()");

  string path;

  for (CTVariableIterator var = (CTVariableIterator)variable.begin();
    var != variable.end(); var++)
  {
    if ((*var)->variable == "PATH")
    {
      path = (*var)->value;
    }
  }
  return path;
}

