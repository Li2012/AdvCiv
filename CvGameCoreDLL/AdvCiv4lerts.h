#pragma once

#ifndef ADVCIV4LERTS_H
#define ADVCIV4LERTS_H

#include "CvString.h"

/*  <advc.210>: I can't write this in Python, so I'm having Civ4lerts.py call
	this class. Will need to make changes in Civ4lerts.py, Civ4lerts.xml,
	BugAlertsOptionsTab.py, CvPlayer.h, CvPlayer.cpp and some file for game text
	(like CIV4GameText_advc.xml) to add more alerts. */
class AdvCiv4lert {

public:
	AdvCiv4lert();
	void init(PlayerTypes ownerId);
	// Silent calls are for initializing data after loading a savegame
	void check(bool silent);

protected:
	// Subclasses should overwrite this ...
	virtual void check()=0;
	// and this, probably
	virtual void reset();
	void msg(CvWString s, LPCSTR icon = NULL, int x = -1, int y = -1,
			int goodOrBad = 0) const;
	PlayerTypes ownerId;
private:
	bool isSilent;
};

// <advc.210a>
class WarTradeAlert : public AdvCiv4lert {
public:
	WarTradeAlert();
protected:
	void check();
	void reset();
private:
	bool willWar[MAX_CIV_TEAMS][MAX_CIV_TEAMS];
}; // </advc.210a>

// <advc.210b>
class RevoltAlert : public AdvCiv4lert {
public:
	RevoltAlert();
protected:
	void check();
	void reset();
private:
	std::set<int> revoltPossible;
	std::set<int> occupation;
}; // </advc.210b>

// </advc.210>

#endif
