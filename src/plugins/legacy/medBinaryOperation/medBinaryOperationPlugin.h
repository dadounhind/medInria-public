/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCoreSupport/dtkPlugin.h>

#include "medBinaryOperationPluginExport.h"

class MEDBINARYOPERATIONPLUGIN_EXPORT medBinaryOperationPlugin : public dtkPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "fr.inria.medBinaryOperationPlugin" FILE "medBinaryOperationPlugin.json")
    Q_INTERFACES(dtkPlugin)
    
public:
    medBinaryOperationPlugin(QObject *parent = 0);
    virtual bool initialize();
    
    virtual QString name() const;
    virtual QString description() const;
    virtual QString version() const;
    virtual QStringList types() const;

    //TODO class medPlugin
    virtual bool uninitialize();
    virtual QStringList tags(void) const;
};
