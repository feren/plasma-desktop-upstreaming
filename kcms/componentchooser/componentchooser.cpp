/***************************************************************************
 *   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>                  *
 *   Copyright (C) 2020 Méven Car <meven.car@kdemail.net>                  *
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "componentchooser.h"

#include <KService>
#include <KOpenWithDialog>
#include <QDBusConnection>
#include <QDBusMessage>
#include <KApplicationTrader>
#include <KSharedConfig>
#include <KConfigGroup>

ComponentChooser::ComponentChooser(QObject *parent, const QString &mimeType, const QString &type, const QString &defaultApplication)
    : QObject(parent)
    , m_mimeType(mimeType)
    , m_type(type)
    , m_defaultApplication(defaultApplication)
{
}

bool ComponentChooser::defaults()
{
    if (m_defaultIndex)
        return select(*m_defaultIndex, false);
    return false;
}

void ComponentChooser::load()
{
    bool preferredServiceAdded = false;

    KService::Ptr preferredService = KApplicationTrader::preferredService(m_mimeType);

    KApplicationTrader::query([&preferredServiceAdded, preferredService, this](const KService::Ptr &service) {
        if (service->exec().isEmpty() || !service->categories().contains(m_type) || (!service->serviceTypes().contains(m_mimeType))) {
            return false;
        }
        QVariantMap application;
        application["name"] = service->name();
        application["icon"] = service->icon();
        application["storageId"] = service->storageId();
        m_applications += application;
        if ((preferredService && preferredService->storageId() == service->storageId())) {
            m_index = m_applications.length() - 1;
            preferredServiceAdded = true;
        }
        if (service->storageId() == m_defaultApplication) {
            m_defaultIndex = m_applications.length() - 1;
        }
        return false;
    });
    if (preferredService && !preferredServiceAdded) {
        // standard application was specified by the user
        QVariantMap application;
        application["name"] = preferredService->name();
        application["icon"] = preferredService->icon();
        application["storageId"] = preferredService->storageId();
        m_applications += application;
        m_index = m_applications.length() - 1;
    }
    QVariantMap application;
    application["name"] = QStringLiteral("Other...");
    application["icon"] = QStringLiteral("application-x-shellscript");
    application["storageId"] = QStringLiteral("");
    m_applications += application;
    if (m_index == -1)
        m_index = 0;
    Q_EMIT applicationsChanged();
    Q_EMIT indexChanged();
}

bool ComponentChooser::select(int index, bool rebuildCache)
{
    if (m_index == index)
        return false;
    if (index == m_applications.length() - 1) {
        KOpenWithDialog *dialog = new KOpenWithDialog(m_mimeType, QString());
        dialog->setSaveNewApplications(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &KOpenWithDialog::finished, this, [rebuildCache, this, dialog] (int result) {
            if(result != QDialog::Accepted) {
                Q_EMIT indexChanged();
                return;
            }

            const auto service = dialog->service();
            // Check if the selected application is already in the list
            for (int i = 0; i < m_applications.length(); i++) {
                if (m_applications[i].toMap()["storageId"] == service->storageId()) {
                    m_index = i;
                    save(rebuildCache);
                    Q_EMIT indexChanged();
                    return;
                }
            }
            const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
            QVariantMap application;
            application["name"] = service->name();
            application["icon"] = icon;
            application["storageId"] = service->storageId();
            m_applications.insert(m_applications.length() - 1, application);
            m_index = m_applications.length() - 2;
            save(rebuildCache);
            Q_EMIT applicationsChanged();
            Q_EMIT indexChanged();
        });
        dialog->open();
        return false;
    } else {
        m_index = index;
    }
    save(rebuildCache);
    Q_EMIT indexChanged();
    return true;
}

void ComponentChooser::save(const QString &mime, const QString &storageId)
{
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (profile->isConfigWritable(true)) {
        KConfigGroup defaultApp(profile, "Default Applications");
        defaultApp.writeXdgListEntry(mime, QStringList(storageId));

        KConfigGroup addedApps(profile, QStringLiteral("Added Associations"));
        QStringList apps = addedApps.readXdgListEntry(mime);
        apps.removeAll(storageId);
        apps.prepend(storageId); // make it the preferred app, i.e first in list
        addedApps.writeXdgListEntry(mime, apps);
        profile->sync();

        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"), QStringLiteral("/KLauncher"), QStringLiteral("org.kde.KLauncher"), QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }
}
