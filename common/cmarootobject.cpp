/*
 *  QCMA: Cross-platform content manager assistant for the PS Vita
 *
 *  Copyright (C) 2013  Codestation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmarootobject.h"

#include <QDir>
#include <QMap>

QString CMARootObject::uuid = "ffffffffffffffff";

CMARootObject::CMARootObject(int ohfi) :
    num_filters(0), filters(NULL), root_ohfi(ohfi)
{
}

void CMARootObject::initObject(const QString &path)
{
    metadata.ohfi = root_ohfi;
    metadata.type = VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_REGULAR;

    switch(root_ohfi) {
    case VITA_OHFI_MUSIC:
        metadata.dataType = Music;
        m_path = path;
        num_filters = 5;
        filters = new metadata_t[5];
        createFilter(&filters[0], "Artists", VITA_DIR_TYPE_MASK_MUSIC | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_ARTISTS);
        createFilter(&filters[1], "Albums", VITA_DIR_TYPE_MASK_MUSIC | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_ALBUMS);
        createFilter(&filters[2], "Songs", VITA_DIR_TYPE_MASK_MUSIC | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_SONGS);
        createFilter(&filters[3], "Genres", VITA_DIR_TYPE_MASK_MUSIC | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_GENRES);
        createFilter(&filters[4], "Playlists", VITA_DIR_TYPE_MASK_MUSIC | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_PLAYLISTS);
        break;

    case VITA_OHFI_PHOTO:
        metadata.dataType = Photo;
        m_path = path;
        num_filters = 3;
        filters = new metadata_t[3];
        createFilter(&filters[0], "All", VITA_DIR_TYPE_MASK_PHOTO | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_ALL);
        createFilter(&filters[1], "Month", VITA_DIR_TYPE_MASK_PHOTO | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_MONTH);
        createFilter(&filters[2], "Folders", VITA_DIR_TYPE_MASK_PHOTO | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_REGULAR);
        break;

    case VITA_OHFI_VIDEO:
        metadata.dataType = Video;
        m_path = path;
        num_filters = 2;
        filters = new metadata_t[2];
        createFilter(&filters[0], "All", VITA_DIR_TYPE_MASK_VIDEO | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_ALL);
        createFilter(&filters[1], "Folders", VITA_DIR_TYPE_MASK_VIDEO | VITA_DIR_TYPE_MASK_ROOT | VITA_DIR_TYPE_MASK_REGULAR);
        break;

    case VITA_OHFI_VITAAPP:
        metadata.dataType = App;
        m_path = QDir(QDir(path).absoluteFilePath("APP")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_PSPAPP:
        metadata.dataType = App;
        m_path = QDir(QDir(path).absoluteFilePath("PGAME")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_PSPSAVE:
        metadata.dataType = SaveData;
        m_path = QDir(QDir(path).absoluteFilePath("PSAVEDATA")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_PSXAPP:
        metadata.dataType = App;
        m_path = QDir(QDir(path).absoluteFilePath("PSGAME")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_PSMAPP:
        metadata.dataType = App;
        m_path = QDir(QDir(path).absoluteFilePath("PSM")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_BACKUP:
        metadata.dataType = App;
        m_path = QDir(QDir(path).absoluteFilePath("SYSTEM")).absoluteFilePath(uuid);
        num_filters = 0;
        break;

    case VITA_OHFI_PACKAGE:
        metadata.dataType = Package;
        m_path = path;
        num_filters = 0;
    }

    // create the backup directories
    QDir dir(m_path);
    dir.mkpath(dir.absolutePath());
}

CMARootObject::~CMARootObject()
{
    for(int i = 0; i < num_filters; i++) {
        free(filters[i].name);
        free(filters[i].path);
    }

    delete[] filters;
}

void CMARootObject::createFilter(metadata_t *filter, const char *name, int type)
{
    filter->ohfiParent = metadata.ohfi;
    filter->ohfi = ohfi_count++;
    filter->name = strdup(name);
    filter->path = strdup(metadata.path ? metadata.path : "");
    filter->type = type;
    filter->dateTimeCreated = 0;
    filter->size = 0;
    filter->dataType = static_cast<DataType>(Folder | Special);
    filter->next_metadata = NULL;
    //qDebug("Added filter %s to database with OHFI %d (%s)", name, filter->ohfi, metadata.name);
}

// kind is VITA_DIR_TYPE_MASK_ARTISTS or VITA_DIR_TYPE_MASK_ALBUMS
// ponytail: albums are grouped by name only, so same-named albums by different artists merge
void CMARootObject::appendGroups(QList<CMAObject *> &list, int kind)
{
    int filter_ohfi = 0;

    for(int i = 0; i < num_filters; i++) {
        if((filters[i].type & 0xFFFF) == kind) {
            filter_ohfi = filters[i].ohfi;
        }
    }

    if(!filter_ohfi) {
        return;
    }

    // QMap keeps the groups sorted by name
    QMap<QString, qint64> groups;

    foreach(const CMAObject *obj, list) {
        if(MASK_SET(obj->metadata.dataType, Music | File)) {
            groups[obj->groupName(kind)] += obj->metadata.size;
        }
    }

    // appended after the scan, so the ohfis keep the list sorted for lower_bound lookups
    for(QMap<QString, qint64>::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it) {
        CMAObject *group = new CMAObject(this);
        // copies from the Vita into a group land in the music root, like filters
        group->m_path = m_path;
        group->metadata.ohfiParent = filter_ohfi;
        group->metadata.ohfi = ohfi_count++;
        group->metadata.name = strdup(it.key().toUtf8().constData());
        group->metadata.path = strdup(metadata.path ? metadata.path : "");
        group->metadata.type = VITA_DIR_TYPE_MASK_MUSIC | kind;
        group->metadata.size = it.value();
        group->metadata.dataType = static_cast<DataType>(Music | Folder | Special);
        list << group;
    }

    qDebug("Added %i groups of kind 0x%X", static_cast<int>(groups.size()), kind);
}

int CMARootObject::getFilters(metadata_t **p_head)
{
    int numObjects = num_filters;

    for(int i = 0; i < numObjects; i++) {
        filters[i].next_metadata = &filters[i + 1];
    }

    filters[numObjects - 1].next_metadata = NULL;

    if(p_head != NULL) {
        *p_head = filters;
    }

    return numObjects;
}
