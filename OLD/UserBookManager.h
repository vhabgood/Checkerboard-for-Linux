#pragma once

#include "checkers_types.h"

#define MAXUSERBOOK 1024 // Define MAXUSERBOOK here, as it's specific to userbook

struct userbookentry {
	pos position;
	CBmove move;
};

// UserBookManager class definition will go here
class UserBookManager {
public:
    UserBookManager();

    void loadUserBook(const QString& filename);
    void saveUserBook(const QString& filename);
    void addMoveToUserBook(const Board8x8 board, const CBmove& move);
    void deleteCurrentEntry();
    void navigateToNextEntry();
    void navigateToPreviousEntry();
    bool lookupMove(const Board8x8 board, int color, int gametype, CBmove* bookMove) const;
    void resetNavigation();

    int getUserBookNum() const { return m_userbooknum; }
    int getUserBookCur() const { return m_userbookcur; }
    const userbookentry* getCurrentEntry() const;

private:
    userbookentry m_userbook[MAXUSERBOOK];
    int m_userbooknum;
    int m_userbookcur;
};