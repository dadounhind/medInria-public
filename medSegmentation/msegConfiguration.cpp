// /////////////////////////////////////////////////////////////////
// Generated by dtkPluginGenerator
// /////////////////////////////////////////////////////////////////

#include "msegConfiguration.h"

#include "msegToolbox.h"

#include "medAbstractViewScene.h"
#include "msegController.h"

#include <medCore/medAbstractView.h>

#include <medGui/medStackedViewContainers.h>
#include <medGui/medProgressionStack.h>
#include <medGui/medViewerConfigurationFactory.h>
#include <medGui/medViewContainer.h>
#include <medGui/medViewerToolBoxView.h>

#include <dtkCore/dtkLog.h>

namespace mseg {
// /////////////////////////////////////////////////////////////////
// ConfigurationPrivate
// /////////////////////////////////////////////////////////////////
class ConfigurationPrivate
{
public:
    medViewerToolBoxView                *viewToolBox;
    Toolbox          *segmentationToolBox;

    QScopedPointer<Controller> controller;
};

// /////////////////////////////////////////////////////////////////
// Configuration
// /////////////////////////////////////////////////////////////////
//static
medViewerConfiguration * Configuration::createMedSegmentationConfiguration()
{
    return new Configuration;
}

Configuration::Configuration(QWidget * parent /* = NULL */ ) : 
medViewerConfiguration(parent), d(new ConfigurationPrivate)
{
    d->segmentationToolBox = NULL;
    d->controller.reset( new Controller( this ) );
}

Configuration::~Configuration(void)
{
    delete d;
    d = NULL;
}

bool Configuration::registerWithViewerConfigurationFactory()
{
    return medViewerConfigurationFactory::instance()->registerConfiguration( 
        Configuration::ConfigurationName(), 
        Configuration::createMedSegmentationConfiguration
        );
}

QString Configuration::ConfigurationName()
{
    // This is the string that appears in the drop-down.
    static QString confName( tr("Segmentation") );
    return confName;
}

void Configuration::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        //Containers:
        addSingleContainer();
        // addMultiContainer();
        // addCustomContainer();
        connect(stackedViewContainers()->container("Single"),SIGNAL(viewAdded(dtkAbstractView*)),
            this,SLOT(onViewAdded(dtkAbstractView*)));
        connect(stackedViewContainers()->container("Single"),SIGNAL(viewRemoved(dtkAbstractView*)),
            this,SLOT(onViewRemoved(dtkAbstractView*)));
    }
}

//static
QString Configuration::description( void ) const
{
    static QString descString( "msegConfiguration" );
    return descString;
}

void Configuration::onViewAdded( dtkAbstractView* view )
{
    emit viewAdded(view);
}

void Configuration::onViewRemoved( dtkAbstractView* view )
{
    emit viewRemoved(view);
}


medProgressionStack * Configuration::progressionStack()
{
    return d->segmentationToolBox->progressionStack();
}

void Configuration::buildConfiguration(  )
{
    QWidget * parent = qobject_cast<QWidget *>(this->parent());
    if ( !d->segmentationToolBox)
        d->segmentationToolBox = new Toolbox( parent );

    this->addToolBox( d->segmentationToolBox );
}

Toolbox * Configuration::segmentationToobox()
{
    return d->segmentationToolBox;
}



} // namespace mseg

