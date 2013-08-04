/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,2007 crypton
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

#include <retroshare/rstypes.h>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionProgressBarV2>
#include <QProgressBar>
#include <QApplication>

#include "DLListDelegate.h"

Q_DECLARE_METATYPE(FileProgressInfo)

DLListDelegate::DLListDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
	;
}

DLListDelegate::~DLListDelegate(void)
{
	;
}

void DLListDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QString byteUnits[4] = {tr("B"), tr("KB"), tr("MB"), tr("GB")};
	QStyleOptionViewItem opt = option;
	QStyleOptionProgressBarV2 newopt;
	QRect pixmapRect;
	QPixmap pixmap;
	qlonglong fileSize;
	double dlspeed, multi;
	int seconds,minutes, hours, days;
	qlonglong remaining;
	QString temp , status;
	qlonglong completed;
	qlonglong downloadtime;

	// prepare
	painter->save();
	painter->setClipRect(opt.rect);

	//set text color
        QVariant value = index.data(Qt::TextColorRole);
        if(value.isValid() && qvariant_cast<QColor>(value).isValid()) {
                opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));
        }
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        if(option.state & QStyle::State_Selected){
                painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        } else {
                painter->setPen(opt.palette.color(cg, QPalette::Text));
        }

        // draw the background color if not the progress column or if progress is not displayed
        if(index.column() != COLUMN_PROGRESS) {
                if(option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
                        if(cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
                                cg = QPalette::Inactive;
                        }
                        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
                } else {
                        value = index.data(Qt::BackgroundRole);
                        if(value.isValid() && qvariant_cast<QColor>(value).isValid()) {
                                painter->fillRect(option.rect, qvariant_cast<QColor>(value));
                        }
                }
        }
	switch(index.column()) {
        case COLUMN_SIZE:
			fileSize = index.data().toLongLong();
                        if(fileSize <= 0){
                                temp = "";
			} else {
				multi = 1.0;
				for(int i = 0; i < 5; ++i) {
					if (fileSize < 1024) {
						fileSize = index.data().toLongLong();
						temp.sprintf("%.2f ", fileSize / multi);
						temp += byteUnits[i];
						break;
					}
					fileSize /= 1024;
					multi *= 1024.0;
				}
			}
			painter->drawText(option.rect, Qt::AlignRight, temp);
			break;
        case COLUMN_REMAINING:
			remaining = index.data().toLongLong();
			if(remaining <= 0){
        temp = "";
			} else {
				multi = 1.0;
				for(int i = 0; i < 5; ++i) {
					if (remaining < 1024) {
						remaining = index.data().toLongLong();
						temp.sprintf("%.2f ", remaining / multi);
						temp += byteUnits[i];
						break;
					}
					remaining /= 1024;
					multi *= 1024.0;
				}
			}
			painter->drawText(option.rect, Qt::AlignRight, temp);
			break;
        case COLUMN_COMPLETED:
			completed = index.data().toLongLong();
                        if(completed <= 0){
                                temp = "";
			} else {
				multi = 1.0;
				for(int i = 0; i < 5; ++i) {
					if (completed < 1024) {
						completed = index.data().toLongLong();
						temp.sprintf("%.2f ", completed / multi);
						temp += byteUnits[i];
						break;
					}
					completed /= 1024;
					multi *= 1024.0;
				}
			}
			painter->drawText(option.rect, Qt::AlignRight, temp);
			break;
        case COLUMN_DLSPEED:
                        dlspeed = index.data().toDouble();
                        if (dlspeed <= 0) {
                            temp = "";
                        } else {
                            temp.clear();
                            temp.sprintf("%.2f", dlspeed/1024.);
                            temp += " KB/s";
                        }
			painter->drawText(option.rect, Qt::AlignRight, temp);
			break;
        case COLUMN_PROGRESS:
			{
				// create a xProgressBar
				FileProgressInfo pinfo = index.data().value<FileProgressInfo>() ;

//				std::cerr << "drawing progress info: nb_chunks = " << pinfo.nb_chunks ;
//				for(uint i=0;i<pinfo.cmap._map.size();++i)
//					std::cerr << pinfo.cmap._map[i] << " " ;
//				std::cerr << std::endl ;
				
				painter->save() ;
				xProgressBar progressBar(pinfo,option.rect, painter); // the 3rd param is the  color schema (0 is the default value)
				if(pinfo.type == FileProgressInfo::DOWNLOAD_LINE)
				{
					progressBar.setDisplayText(true); // should display % text?
					progressBar.setColorSchema(0) ;
				}
				else
				{
					progressBar.setDisplayText(false); // should display % text?
					progressBar.setColorSchema(1) ;
				}
				progressBar.setVerticalSpan(1);
				progressBar.paint(); // paint the progress bar

				painter->restore() ;
			}
			painter->drawText(option.rect, Qt::AlignCenter, newopt.text);
			break;
        case COLUMN_DOWNLOADTIME:
			downloadtime = index.data().toLongLong();
			minutes = downloadtime / 60;
			seconds = downloadtime % 60;
			hours = minutes / 60;
			minutes = minutes % 60 ;
			days = hours / 24;
			hours = hours % 24 ;
			if(days > 0) {
				temp = QString::number(days)+"d "+QString::number(hours)+"h" ;
			} else if(hours > 0 || days > 0) {
				temp = QString::number(hours)+"h "+QString::number(minutes)+"m" ;
			} else if(minutes > 0 || hours > 0) {
				temp = QString::number(minutes)+"m"+QString::number(seconds)+"s" ;
			} else if(seconds > 0) {
				temp = QString::number(seconds)+"s" ;
			} else
				temp = "" ;
			painter->drawText(option.rect, Qt::AlignCenter, temp);
			break;
        case COLUMN_NAME:
        		// decoration
                        value = index.data(Qt::DecorationRole);
                        temp = index.data().toString();
                        pixmap = qvariant_cast<QIcon>(value).pixmap(option.decorationSize, option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, option.state & QStyle::State_Open ? QIcon::On : QIcon::Off);
                        pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0): QRect(QPoint(0, 0), option.decorationSize));
                        if (pixmapRect.isValid()){
                                QPoint p = QStyle::alignedRect(option.direction, Qt::AlignLeft, pixmap.size(), option.rect).topLeft();
                                painter->drawPixmap(p, pixmap);
                                temp = " " + temp;
                        }
                        painter->drawText(option.rect.translated(pixmap.size().width(), 0), Qt::AlignLeft, temp);
                        break;
		default:
			painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
	}

	// done
	painter->restore();
}

QSize DLListDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	return QSize(50,17);
	QVariant value = index.data(Qt::FontRole);
	QFont fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
	QFontMetrics fontMetrics(fnt);
	const QString text = index.data(Qt::DisplayRole).toString();
	QRect textRect = QRect(0, 0, 0, fontMetrics.lineSpacing() * (text.count(QLatin1Char('\n')) + 1));
	return textRect.size();
}
