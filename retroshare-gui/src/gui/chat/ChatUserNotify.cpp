/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2012 RetroShare Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "ChatUserNotify.h"
#include "gui/notifyqt.h"
#include "gui/MainWindow.h"
#include "gui/chat/ChatDialog.h"
#include "gui/settings/rsharesettings.h"

#include <retroshare/rsnotify.h>
#include <retroshare/rsmsgs.h>

ChatUserNotify::ChatUserNotify(QObject *parent) :
    UserNotify(parent)
{
    connect(NotifyQt::getInstance(), SIGNAL(chatMessageReceived(ChatMessage)), this, SLOT(chatMessageReceived(ChatMessage)));
}

bool ChatUserNotify::hasSetting(QString *name, QString *group)
{
	if (name) *name = tr("Private Chat");
	if (group) *group = "PrivateChat";

	return true;
}

QIcon ChatUserNotify::getIcon()
{
	return QIcon(":/images/chat.png");
}

QIcon ChatUserNotify::getMainIcon(bool hasNew)
{
	return hasNew ? QIcon(":/images/user/friends24_notify.png") : QIcon(":/images/groupchat.png");
}

unsigned int ChatUserNotify::getNewCount()
{
    int sum = 0;
    for(std::map<ChatId, int>::iterator mit = mWaitingChats.begin(); mit != mWaitingChats.end(); ++mit)
    {
        sum += mit->second;
    }
    return sum;
}

void ChatUserNotify::iconClicked()
{
	ChatDialog *chatDialog = NULL;
    if (mWaitingChats.empty() == false) {
        chatDialog = ChatDialog::getChat(mWaitingChats.begin()->first, RS_CHAT_OPEN | RS_CHAT_FOCUS);
        mWaitingChats.erase(mWaitingChats.begin());
	}

	if (chatDialog == NULL) {
		MainWindow::showWindow(MainWindow::Friends);
	}
    updateIcon();
}

void ChatUserNotify::chatMessageReceived(ChatMessage msg)
{
    if(ChatDialog::getExistingChat(msg.chat_id) || (Settings->getChatFlags() & RS_CHAT_OPEN) || msg.chat_id.isGxsId())
        ChatDialog::chatMessageReceived(msg);
    else
    {
        // this implicitly counts broadcast messages, because broadcast messages are not handled by chat dialog
        bool found = false;
        for(std::map<ChatId, int>::iterator mit = mWaitingChats.begin(); mit != mWaitingChats.end(); ++mit)
        {
            if(msg.chat_id.isSameEndpoint(mit->first))
            {
                mit->second++;
                found = true;
            }
        }
        if(!found)
            mWaitingChats[msg.chat_id] = 1;
        updateIcon();
    }
}
