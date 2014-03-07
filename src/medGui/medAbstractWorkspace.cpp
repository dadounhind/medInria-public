/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medAbstractWorkspace.h>

#include <QUuid>

#include <medDataIndex.h>
#include <medSettingsManager.h>
#include <medToolBox.h>
#include <medViewContainer.h>
#include <medViewContainerManager.h>
#include <medTabbedViewContainers.h>
#include <medAbstractView.h>
#include <medAbstractLayeredView.h>
#include <medToolBoxHeader.h>
#include <medAbstractInteractor.h>
#include <medMetaDataKeys.h>
#include <medParameterPool.h>
#include <medParameterPoolManager.h>
#include <medAbstractParameter.h>
#include <medDataManager.h>

class medAbstractWorkspacePrivate
{
public:
    QWidget *parent;

    QString name;

    bool databaseVisibility;
    bool toolBoxesVisibility;

    medTabbedViewContainers * viewContainerStack;
    QHash <QListWidgetItem*, QUuid> containerForLayerWidgetsItem;

    QMultiHash <QString, QPair<medAbstractView*,int> > layersPool;
    QHash<int, QComboBox*> poolSelectors;

    QList<medToolBox*> toolBoxes;
    medToolBox *selectionToolBox;
    medToolBox *layerListToolBox;
    medToolBox *interactorToolBox;
    medToolBox *navigatorToolBox;
    QListWidget* layerListWidget;

    QList<QListWidgetItem*> selectedLayers;

    medParameterPool *temporaryPoolForInteractors;
};

medAbstractWorkspace::medAbstractWorkspace(QWidget *parent) : QObject(parent), d(new medAbstractWorkspacePrivate)
{
    d->parent = parent;

    d->selectionToolBox = new medToolBox;
    d->selectionToolBox->setTitle("Selection");
    d->selectionToolBox->header()->hide();
    d->selectionToolBox->hide();

    d->viewContainerStack = new medTabbedViewContainers(parent);
    connect(d->viewContainerStack, SIGNAL(containersSelectedChanged()), this, SLOT(updateNavigatorsToolBox()));
    connect(d->viewContainerStack, SIGNAL(currentLayerChanged()), this, SLOT(updateInteractorsToolBox()));

    d->databaseVisibility = true;
    d->toolBoxesVisibility = true;

    d->navigatorToolBox = new medToolBox;
    d->navigatorToolBox->setTitle("View settings");
    d->navigatorToolBox->hide();
    d->selectionToolBox->addWidget(d->navigatorToolBox);

    d->layerListToolBox = new medToolBox;
    d->layerListToolBox->setTitle("Layer settings");
    d->layerListToolBox->hide();
    d->selectionToolBox->addWidget(d->layerListToolBox);

    d->interactorToolBox = new medToolBox;
    d->interactorToolBox->setTitle("Interactors");
    d->interactorToolBox->header()->hide();
    d->interactorToolBox->hide();
    d->selectionToolBox->addWidget(d->interactorToolBox);

    d->layerListToolBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    d->layerListWidget = NULL;

    d->temporaryPoolForInteractors = new medParameterPool(this);
}

medAbstractWorkspace::~medAbstractWorkspace(void)
{
    delete d;
    d = NULL;
}

void medAbstractWorkspace::addToolBox(medToolBox *toolbox)
{
    d->toolBoxes.append(toolbox);
}

void medAbstractWorkspace::removeToolBox(medToolBox *toolbox)
{
    d->toolBoxes.removeOne(toolbox);
}

QList<medToolBox*> medAbstractWorkspace::toolBoxes() const
{
    return d->toolBoxes;
}

medToolBox* medAbstractWorkspace::selectionToolBox() const
{
    return d->selectionToolBox;
}

void medAbstractWorkspace::setDatabaseVisibility(bool visibility)
{
    d->databaseVisibility = visibility;
}

bool medAbstractWorkspace::isDatabaseVisible(void) const
{
    return d->databaseVisibility;
}

medTabbedViewContainers* medAbstractWorkspace::stackedViewContainers() const
{
    return d->viewContainerStack;
}

void medAbstractWorkspace::clear()
{
    this->setupViewContainerStack();
    this->clearWorkspaceToolBoxes();
    return;
}

void medAbstractWorkspace::setToolBoxesVisibility (bool value)
{
    d->toolBoxesVisibility = value;
}

bool medAbstractWorkspace::areToolBoxesVisible() const
{
    return d->toolBoxesVisibility;
}


void medAbstractWorkspace::clearWorkspaceToolBoxes()
{
    foreach(medToolBox* tb,d->toolBoxes)
        tb->clear();
}

void medAbstractWorkspace::addNewTab()
{
    d->viewContainerStack->addContainerInTab(this->identifier());
}

void medAbstractWorkspace::setName(QString &name)
{
    d->name = name;
}

QString& medAbstractWorkspace::name() const
{
    return d->name;
}

void medAbstractWorkspace::updateNavigatorsToolBox()
{
    d->navigatorToolBox->clear();

    QList<QWidget*>  navigators;
    QStringList viewType;
    foreach(QUuid uuid, d->viewContainerStack->containersSelected())
    {
        medViewContainer *container = medViewContainerManager::instance()->container(uuid);
        // update the toolbox when the content of the view change
        medAbstractView* view = container->view();
        // add nothing if the view is empty
        if(!view)
            continue;
        // get only one navigator per view type
        if(!viewType.contains(view->identifier()))
        {
            viewType << view->identifier();
            navigators << view->navigatorWidget();
        }
    }
    // add the navigators widgets
    d->navigatorToolBox->show();
    foreach(QWidget* navigator, navigators)
    {
        d->navigatorToolBox->addWidget(navigator);
        navigator->show();
    }

    // update the layer toolbox according to the selected containers
    this->updateLayersToolBox();
}

void medAbstractWorkspace::updateLayersToolBox()
{
    d->layerListToolBox->clear();
    d->containerForLayerWidgetsItem.clear();
    d->selectedLayers.clear();
    d->poolSelectors.clear();

    delete d->layerListWidget;
    d->layerListWidget = new QListWidget;
    d->layerListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->layerListWidget->setFocusPolicy(Qt::NoFocus);

    connect(d->layerListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(changeCurrentLayer(int)));
    connect(d->layerListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateInteractorsToolBox()));

    foreach(QUuid uuid, d->viewContainerStack->containersSelected())
    {
        // fill the layer widget
        medViewContainer *container = medViewContainerManager::instance()->container(uuid);
        medAbstractLayeredView* layeredView = dynamic_cast<medAbstractLayeredView*>(container->view());
        if(layeredView)
        {
            if(d->layerListWidget->count() != 0)
            {
                // add an empty widget to separate layers from different views
                QListWidgetItem * item = new QListWidgetItem;
                item->setSizeHint(QSize(10, 10));
                item->setFlags(Qt::NoItemFlags);
                d->layerListWidget->addItem(item);
            }
            int firstLayerIndex = d->layerListWidget->count();

            for(unsigned int layer = 0; layer < layeredView->layersCount(); ++layer)
            {
                QWidget *layerWidget = new QWidget;
                layerWidget->setObjectName("layerWidget");

                medAbstractData *data = layeredView->data(layer);
                QString thumbPath = medMetaDataKeys::SeriesThumbnail.getFirstValue(data,":icons/layer.png");
                QString name = medMetaDataKeys::SeriesDescription.getFirstValue(data,"<i>no name</i>");

                QHBoxLayout* layout = new QHBoxLayout(layerWidget);
                layout->setContentsMargins(0,0,10,0);

                QPushButton* thumbnailButton = new QPushButton(layerWidget);
                thumbnailButton->setProperty("row", d->layerListWidget->count());
                QIcon thumbnailIcon;
                // Set the off icon to the greyed out version of the regular icon
                thumbnailIcon.addPixmap(QPixmap(thumbPath), QIcon::Normal, QIcon::On);
                QStyleOption opt(0);
                opt.palette = QApplication::palette();
                QPixmap pix = QApplication::style()->generatedIconPixmap(QIcon::Disabled, QPixmap(thumbPath), &opt);
                thumbnailIcon.addPixmap(pix, QIcon::Normal, QIcon::Off);
                thumbnailButton->setFocusPolicy(Qt::NoFocus);
                thumbnailButton->setIcon(thumbnailIcon);
                thumbnailButton->setIconSize(QSize(22,22));
                thumbnailButton->setCheckable(true);
                thumbnailButton->setChecked(true);
                thumbnailButton->setFlat(true);

                QFont myFont;
                QFontMetrics fm(myFont);
                //TODO: could be nice to elide according to current width (update when resize)
                QString text = fm.elidedText(name, Qt::ElideRight, 100);
                QLabel *layerName = new QLabel("<font color='Black'>"+text+"</font>", layerWidget);
                layerName->setToolTip(name);

                QComboBox *poolSelector = new QComboBox;
                poolSelector->setProperty("row", d->layerListWidget->count());
                poolSelector->addItem("");
                poolSelector->addItem(createIcon("orange"), "4", "orange");
                poolSelector->addItem(createIcon("#0080C0"), "5", "#0080C0");
                poolSelector->addItem(createIcon("yellow"), "6", "yellow");

                QString pool = d->layersPool.key(QPair<medAbstractView*, int>(layeredView, layer));
                poolSelector->setCurrentIndex(poolSelector->findText(pool));

                QString tooltip = QString(tr("Link Layer properties( "));
                foreach (medAbstractInteractor *interactor, layeredView->interactors(layer))
                    foreach(medAbstractParameter* param, interactor->parameters())
                      tooltip += param->name() + ", ";
                tooltip += ")";

                poolSelector->setToolTip(tooltip);
                d->poolSelectors.insert(d->layerListWidget->count(), poolSelector);

                connect(poolSelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateParameterPool(QString)));

                QPushButton *removeButton = new QPushButton;
                removeButton->setProperty("row", d->layerListWidget->count());
                removeButton->setIcon(QIcon(":/icons/cross.svg"));
                removeButton->setIconSize(QSize(15,15));
                removeButton->setFixedSize(20,20);
                //TODO - get possible to remove layer 0 - RDE
                if(layer == 0 && layeredView->layersCount() > 1)
                    removeButton->setDisabled(true);

                layout->addWidget(thumbnailButton);
                layout->addWidget(layerName);
                layout->addStretch();
                foreach (medAbstractInteractor *interactor, layeredView->interactors(layer))
                    layout->addWidget(interactor->layerWidget());

                layout->addWidget(poolSelector);
                layout->addWidget(removeButton);

                connect(thumbnailButton, SIGNAL(clicked(bool)), this, SLOT(setLayerVisibility(bool)));
                connect(removeButton, SIGNAL(clicked()), this, SLOT(removeLayer()));

                layerWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
                layerWidget->resize(d->selectionToolBox->width(), 25);

                QListWidgetItem * item = new QListWidgetItem;
                item->setData(Qt::UserRole, layer);
                d->containerForLayerWidgetsItem.insert(item, uuid);
                item->setSizeHint(QSize(layerWidget->width(), 25));
                d->layerListWidget->addItem(item);
                d->layerListWidget->setItemWidget(item, layerWidget);
                layerWidget->setFocusPolicy(Qt::NoFocus);
            }

            d->layerListWidget->setCurrentRow(firstLayerIndex + layeredView->currentLayer());
        }
    }
    // add the layer widgets
    d->layerListToolBox->show();
    d->layerListToolBox->addWidget(d->layerListWidget);
    d->layerListWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    d->layerListWidget->show();

    this->updateInteractorsToolBox();
}


void medAbstractWorkspace::changeCurrentLayer(int row)
{
    QListWidgetItem* item = d->layerListWidget->item(row);
    QUuid uuid = d->containerForLayerWidgetsItem.value(item);
    medViewContainer* container = medViewContainerManager::instance()->container(uuid);
    if(!container)
        return;

    medAbstractLayeredView *layeredView = dynamic_cast<medAbstractLayeredView*>(container->view());
    if(!layeredView)
        return;

    int currentLayer = item->data(Qt::UserRole).toInt();
    layeredView->setCurrentLayer(currentLayer);

    d->selectedLayers = d->layerListWidget->selectedItems();

    this->updateInteractorsToolBox();
}

void medAbstractWorkspace::updateInteractorsToolBox()
{
    medViewContainerManager *containerMng =  medViewContainerManager::instance();
    foreach(QUuid uuid, d->viewContainerStack->containersSelected())
    {
        containerMng->container(uuid)->highlight();
    }
    d->interactorToolBox->hide();
    d->interactorToolBox->clear();

    if(!d->layerListWidget)
        return;

    // Hack to always have something selected in the layerListWidget
    // TODO: try to find a better solution...
    if(d->layerListWidget->selectedItems().count() == 0)
    {
        foreach(QListWidgetItem* item, d->selectedLayers)
        {
            d->layerListWidget->blockSignals(true);
            item->setSelected(true);
            d->layerListWidget->blockSignals(false);
        }
    }
    else d->selectedLayers = d->layerListWidget->selectedItems();

    // If we have a multiSelection we just want to colorise the container that have at least one layer selected.
    if(d->layerListWidget->selectedItems().size() > 1)
    {
        foreach(QListWidgetItem* item, d->layerListWidget->selectedItems())
        {
            QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);
            medViewContainer *container = containerMng->container(containerUuid);
            container->highlight("#FF6622");
        }

        buildTemporaryPool();

        return;
    }

    QListWidgetItem* item = d->layerListWidget->currentItem();
    if(!item)
        return;

    QList<QString> interactorsIdentifier;
    QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);
    medViewContainer *container = containerMng->container(containerUuid);
    container->highlight("#FF6622");

    medAbstractLayeredView *view = dynamic_cast<medAbstractLayeredView*>(container->view());

    foreach (medAbstractInteractor* interactor, view->currentInteractors())
    {
        QString interactorIdentifier = interactor->identifier();
        if(!interactorsIdentifier.contains(interactorIdentifier))
        {
            interactorsIdentifier << interactorIdentifier;
            QGroupBox *groupBox = new QGroupBox;
            QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);
            groupBoxLayout->addWidget(interactor->toolBoxWidget());
            d->interactorToolBox->addWidget(groupBox);
            interactor->toolBoxWidget()->show();
        }
    }
    d->interactorToolBox->show();

    buildTemporaryPool();
}

void medAbstractWorkspace::setLayerVisibility(bool visibility)
{
    QPushButton *button = dynamic_cast<QPushButton*>(this->sender());
    if(!button)
        return;

    int row = button->property("row").toInt();
    QListWidgetItem* item = d->layerListWidget->item(row);
    QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);

    int layer = item->data(Qt::UserRole).toInt();

    medAbstractLayeredView *layerView = dynamic_cast<medAbstractLayeredView *>(medViewContainerManager::instance()->container(containerUuid)->view());
    if(!layerView)
        return;

    layerView->setVisibility(visibility, layer);
}

void medAbstractWorkspace::removeLayer()
{
    QPushButton *button = dynamic_cast<QPushButton*>(this->sender());
    if(!button)
        return;

    int row = button->property("row").toInt();
    QListWidgetItem* item = d->layerListWidget->item(row);
    QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);

    int layer = item->data(Qt::UserRole).toInt();

    medAbstractLayeredView *layerView = dynamic_cast<medAbstractLayeredView *>(medViewContainerManager::instance()->container(containerUuid)->view());
    if(!layerView)
        return;

    layerView->removeLayer(layer);
    this->updateLayersToolBox();
}


void medAbstractWorkspace::buildTemporaryPool()
{
    medViewContainerManager *containerMng =  medViewContainerManager::instance();
    d->temporaryPoolForInteractors->clear();

    foreach(QListWidgetItem* item, d->layerListWidget->selectedItems())
    {
        QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);
        medViewContainer *container = containerMng->container(containerUuid);

        medAbstractLayeredView *view = dynamic_cast<medAbstractLayeredView*>(container->view());

        foreach (medAbstractInteractor* interactor, view->currentInteractors())
        {
            d->temporaryPoolForInteractors->append(interactor->parameters());
        }
    }
}

void medAbstractWorkspace::updateParameterPool(QString pool)
{
    QComboBox *combo = dynamic_cast<QComboBox*>(this->sender());
    if(!combo)
        return;

    int row = combo->property("row").toInt();

    QList<int> selectedRows;
    QList<QListWidgetItem*> itemstoUpdate;

    foreach(QListWidgetItem* item, d->layerListWidget->selectedItems())
        selectedRows.append(d->layerListWidget->row(item));

    if(selectedRows.contains(row))
        itemstoUpdate = d->layerListWidget->selectedItems();
    else itemstoUpdate.append(d->layerListWidget->item(row));

    foreach(QListWidgetItem* item, itemstoUpdate)
    {
        QUuid containerUuid = d->containerForLayerWidgetsItem.value(item);
        unsigned int layer = item->data(Qt::UserRole).toInt();
        int itemRow = d->layerListWidget->row(item);

        d->poolSelectors.value(itemRow)->setCurrentIndex( d->poolSelectors.value(itemRow)->findText(pool) );

        medViewContainer* container = medViewContainerManager::instance()->container(containerUuid);
        if(!container)
            break;

        medAbstractLayeredView *layeredView = dynamic_cast<medAbstractLayeredView*>(container->view());
        if(!layeredView)
            break;

        QString key = d->layersPool.key(QPair<medAbstractView*, int>(layeredView, layer));
        if(pool == "" || key != "")
            d->layersPool.remove(key, QPair<medAbstractView*, int>(layeredView, layer));
        if(pool != "")
            d->layersPool.insert(pool, QPair<medAbstractView*, int>(layeredView, layer));

        foreach (medAbstractInteractor* interactor, layeredView->interactors(layer))
        {
            foreach(medAbstractParameter *param, interactor->parameters())
            {
                if(pool == "" || key != "")
                    medParameterPoolManager::instance()->unlinkParameter(param);
                if(pool != "")
                   medParameterPoolManager::instance()->linkParameter(param, pool);
            }
        }
    }
}

QIcon medAbstractWorkspace::createIcon(const QString &colorName) const
{
    QPixmap iconPixmap(10,10);
    iconPixmap.fill(QColor(colorName));
    QIcon itemIcon(iconPixmap);
    return itemIcon;
}

void medAbstractWorkspace::open(const medDataIndex &index)
{
    QList<QUuid> containersSelected = d->viewContainerStack->containersSelected();
    if(containersSelected.size() != 1)
        return;

    medViewContainer *container = medViewContainerManager::instance()->container(containersSelected.first());
    if(index.isValidForSeries())
        container->addData(medDataManager::instance()->data(index));
}


