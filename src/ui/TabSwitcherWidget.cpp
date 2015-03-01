/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "TabSwitcherWidget.h"
#include "Window.h"
#include "../core/WindowsManager.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QMovie>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>

namespace Otter
{

TabSwitcherWidget::TabSwitcherWidget(WindowsManager *manager, QWidget *parent) : QWidget(parent),
	m_windowsManager(manager),
	m_model(new QStandardItemModel(this)),
	m_frame(new QFrame(this)),
	m_tabsView(new QListView(m_frame)),
	m_previewLabel(new QLabel(m_frame)),
	m_loadingMovie(NULL)
{
	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->addWidget(m_frame, 0, Qt::AlignCenter);

	setLayout(mainLayout);
	setAutoFillBackground(false);

	QHBoxLayout *frameLayout = new QHBoxLayout(m_frame);
	frameLayout->addWidget(m_tabsView);
	frameLayout->addWidget(m_previewLabel, 0, Qt::AlignCenter);

	m_frame->setLayout(frameLayout);
	m_frame->setAutoFillBackground(true);
	m_frame->setObjectName(QLatin1String("tabSwitcher"));
	m_frame->setStyleSheet(QStringLiteral("#tabSwitcher {background:%1;border:1px solid #B3B3B3;border-radius:4px;}").arg(palette().color(QPalette::Base).name()));

	m_tabsView->setModel(m_model);
	m_tabsView->setGridSize(QSize(200, 22));
	m_tabsView->setIconSize(QSize(22, 22));
	m_tabsView->setStyleSheet(QLatin1String("border:0;"));

	m_previewLabel->setFixedSize(260, 170);
	m_previewLabel->setAlignment(Qt::AlignCenter);
	m_previewLabel->setStyleSheet(QLatin1String("border:1px solid gray;"));

	connect(m_tabsView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentTabChanged(QModelIndex)));
}

void TabSwitcherWidget::showEvent(QShowEvent *event)
{
	grabKeyboard();

	for (int i = 0; i < m_windowsManager->getWindowCount(); ++i)
	{
		Window *window = m_windowsManager->getWindowByIndex(i);

		if (window)
		{
			QList<QStandardItem*> items;
			items.append(new QStandardItem(window->getIcon(), window->getTitle()));
			items.append(new QStandardItem(window->getLastActivity().toString()));
			items[0]->setData(window->getIdentifier(), Qt::UserRole);

			m_model->appendRow(items);
		}
	}

	m_model->sort(1, Qt::DescendingOrder);

	m_tabsView->setCurrentIndex(m_model->index(0, 0));

	QWidget::showEvent(event);
}

void TabSwitcherWidget::hideEvent(QHideEvent *event)
{
	releaseKeyboard();

	QWidget::hideEvent(event);

	m_model->clear();
}

void TabSwitcherWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Tab)
	{
		selectTab(true);
	}
	else if (event->key() == Qt::Key_Backtab)
	{
		selectTab(false);
	}
	else if (event->key() == Qt::Key_Escape)
	{
		hide();
	}
}

void TabSwitcherWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Control)
	{
		accept();

		event->accept();
	}
}

void TabSwitcherWidget::currentTabChanged(const QModelIndex &index)
{
	Window *window = m_windowsManager->getWindowByIdentifier(index.data(Qt::UserRole).toLongLong());

	if (window)
	{
		if (window->getLoadingState() == LoadedState)
		{
			m_previewLabel->setMovie(NULL);
			m_previewLabel->setPixmap(window->getThumbnail());
		}
		else
		{
			if (!m_loadingMovie)
			{
				m_loadingMovie = new QMovie(QLatin1String(":/icons/loading.gif"), QByteArray(), m_previewLabel);
				m_loadingMovie->start();
			}

			m_previewLabel->setPixmap(QPixmap());
			m_previewLabel->setMovie(m_loadingMovie);

			m_loadingMovie->setSpeed((window->getLoadingState() == LoadingState) ? 100 : 10);
		}
	}
	else
	{
		m_previewLabel->setMovie(NULL);
		m_previewLabel->setPixmap(QPixmap());
	}
}

void TabSwitcherWidget::accept()
{
	m_windowsManager->setActiveWindowByIdentifier(m_tabsView->currentIndex().data(Qt::UserRole).toLongLong());

	hide();
}

void TabSwitcherWidget::selectTab(bool next)
{
	const int currentRow = m_tabsView->currentIndex().row();

	m_tabsView->setCurrentIndex(m_model->index((next ? ((currentRow == (m_model->rowCount() - 1)) ? 0 : (currentRow + 1)) : ((currentRow == 0) ? (m_model->rowCount() - 1) : (currentRow - 1))), 0));
}

}
