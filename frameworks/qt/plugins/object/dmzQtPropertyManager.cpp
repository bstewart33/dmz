#include "dmzQtPropertyManager.h"
#include <dmzRuntimeDefinitions.h>
#include <dmzTypesString.h>
#include <qtpropertymanager.h>

#include <QtCore/QDebug>

namespace {

template <class Value, class PrivateData>
static Value getData(const QMap<const QtProperty *, PrivateData> &propertyMap,
            Value PrivateData::*data,
            const QtProperty *property, const Value &defaultValue = Value()) {

    typedef QMap<const QtProperty *, PrivateData> PropertyToData;
    typedef Q_TYPENAME PropertyToData::const_iterator PropertyToDataConstIterator;
    const PropertyToDataConstIterator it = propertyMap.constFind(property);
    if (it == propertyMap.constEnd())
        return defaultValue;
    return it.value().*data;
}

template <class Value, class PrivateData>
static Value getValue(const QMap<const QtProperty *, PrivateData> &propertyMap,
            const QtProperty *property, const Value &defaultValue = Value()) {

    return getData<Value>(propertyMap, &PrivateData::val, property, defaultValue);
}

};


void
dmz::VectorPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value) {

    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        Vector p = m_values[prop].val;
        p.set_x(value);
        q_ptr->setValue(prop, p);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        Vector p = m_values[prop].val;
        p.set_y(value);
        q_ptr->setValue(prop, p);
    } else if (QtProperty *prop = m_zToProperty.value(property, 0)) {
        Vector p = m_values[prop].val;
        p.set_z(value);
        q_ptr->setValue(prop, p);
    }
}


void
dmz:: VectorPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property) {

    if (QtProperty *pointProp  = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_zToProperty.value(property, 0)) {
        m_propertyToZ[pointProp] = 0;
        m_zToProperty.remove(property);
    }
}


dmz::VectorPropertyManager::VectorPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent) {

    d_ptr = new VectorPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}


dmz::VectorPropertyManager::~VectorPropertyManager () {

    clear();
    delete d_ptr;
}


QtDoublePropertyManager *
dmz::VectorPropertyManager::subDoublePropertyManager() const {

    return d_ptr->m_doublePropertyManager;
}


dmz::Vector
dmz::VectorPropertyManager::value(const QtProperty *property) const {

    return getValue<Vector>(d_ptr->m_values, property);
}


int
dmz::VectorPropertyManager::decimals(const QtProperty *property) const {

    return getData<int>(d_ptr->m_values, &VectorPropertyManagerPrivate::Data::decimals, property, 0);
}


QString
dmz::VectorPropertyManager::valueText(const QtProperty *property) const {

    const VectorPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();
    const Vector v = it.value().val;
    const int dec =  it.value().decimals;
    return QString(tr("[%1, %2, %3]").arg(QString::number(v.get_x(), 'f', dec))
                                 .arg(QString::number(v.get_y(), 'f', dec))
                                 .arg(QString::number(v.get_z(), 'f', dec)));
}


void
dmz::VectorPropertyManager::setValue(QtProperty *property, const Vector &val) {

    const VectorPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    if (it.value().val == val)
        return;

    it.value().val = val;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.get_x());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.get_y());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToZ[property], val.get_z());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}


void
dmz::VectorPropertyManager::setDecimals(QtProperty *property, int prec) {

    const VectorPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    VectorPropertyManagerPrivate::Data data = it.value();

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToZ[property], prec);

    it.value() = data;

    emit decimalsChanged(property, data.decimals);
}


void
dmz::VectorPropertyManager::initializeProperty(QtProperty *property) {

    d_ptr->m_values[property] = VectorPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

    QtProperty *zProp = d_ptr->m_doublePropertyManager->addProperty();
    zProp->setPropertyName(tr("Z"));
    d_ptr->m_doublePropertyManager->setDecimals(zProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(zProp, 0);
    d_ptr->m_propertyToZ[property] = zProp;
    d_ptr->m_zToProperty[zProp] = property;
    property->addSubProperty(zProp);
}


void
dmz::VectorPropertyManager::uninitializeProperty(QtProperty *property) {

    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *zProp = d_ptr->m_propertyToZ[property];
    if (zProp) {
        d_ptr->m_zToProperty.remove(zProp);
        delete zProp;
    }
    d_ptr->m_propertyToZ.remove(property);

    d_ptr->m_values.remove(property);
}

dmz::MaskPropertyManagerPrivate::MaskPropertyManagerPrivate(Definitions &defs) :
      m_settingValue (false),
      m_defs (defs) {;}

void
dmz::MaskPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value) {
   
   if (m_settingValue)
      return;
      
    QtProperty *prop = m_maskToProperty.value(property, 0);
    if (prop == 0) {
        return;

    foreach (QtProperty *p, m_propertyToMasks[prop]) {

       if (p == property) {
           
          Mask state;
          String name (qPrintable (p->propertyName ()));
          m_defs.lookup_state (name, state);
           
          Mask v = m_values[prop];
           
          if (value) { v |= state; }
          else { v.unset (state); }
           
          q_ptr->setValue (prop, v);
          
          return;
       }
    }
}

void
dmz::MaskPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property) {
   
    QtProperty *maskProperty = m_maskToProperty.value(property, 0);
    if (maskProperty == 0)
        return;

    m_propertyToMasks[maskProperty].replace(m_propertyToMasks[maskProperty].indexOf(property), 0);
    m_maskToProperty.remove(property);
}


dmz::MaskPropertyManager::MaskPropertyManager(Definitions &defs, QObject *parent)
    : QtAbstractPropertyManager(parent) {
       
    d_ptr = new MaskPropertyManagerPrivate (defs);
    d_ptr->q_ptr = this;

    d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);
    connect(d_ptr->m_boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotBoolChanged(QtProperty *, bool)));
    connect(d_ptr->m_boolPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

dmz::MaskPropertyManager::~MaskPropertyManager() {
   
    clear();
    delete d_ptr;
}

QtBoolPropertyManager *
dmz::MaskPropertyManager::subBoolPropertyManager() const {
   
    return d_ptr->m_boolPropertyManager;
}


void
dmz::MaskPropertyManager::setValueNames (const QStringList &NameList) {

   d_ptr->m_nameList = NameList;
}


dmz::Mask
dmz::MaskPropertyManager::value(const QtProperty *property) const {
   
    return d_ptr->m_values.value (property, Mask ());
}


QString
dmz::MaskPropertyManager::valueText(const QtProperty *property) const {
   
    const MaskPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
    if (it == d_ptr->m_values.constEnd())
        return QString();

    String name;
    d_ptr->m_defs.lookup_state_name (it.value (), name);

    return name.get_buffer ();
}

void
dmz::MaskPropertyManager::setValue(QtProperty *property, const Mask &val) {
   
    const MaskPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    const Mask oldVal = it.value();
    if (oldVal == val)
        return;

    if (!val.is_set ())
       return;

    it.value() = val;
    
    d_ptr->m_settingValue = true;
      
    foreach (QtProperty *prop, d_ptr->m_propertyToMasks[property]) {
       
       if (prop) {
          
          Mask state;
          String name (qPrintable (prop->propertyName ()));
          d_ptr->m_defs.lookup_state (name, state);
          d_ptr->m_boolPropertyManager->setValue(prop,  val.contains (state));
       }
    }
    
    d_ptr->m_settingValue = false;
    
    emit propertyChanged(property);
    emit valueChanged(property, val);
}


void
dmz::MaskPropertyManager::initializeProperty(QtProperty *property) {
   
    d_ptr->m_values[property] = Mask ();
    
    QList<QtProperty *>list;
    
    foreach (QString name, d_ptr->m_nameList) {
       QtProperty *prop = d_ptr->m_boolPropertyManager->addProperty();
       prop->setPropertyName(name);
       list.append(prop);
       property->addSubProperty(prop);
       d_ptr->m_maskToProperty[prop] = property;
    }
    
    d_ptr->m_propertyToMasks[property] = list;
}


void
dmz::MaskPropertyManager::uninitializeProperty(QtProperty *property) {
   
    foreach (QtProperty *prop, d_ptr->m_propertyToMasks[property]) {
       
        if (prop) {
           d_ptr->m_maskToProperty.remove(prop);
           delete prop;
        }
    }
    
    d_ptr->m_propertyToMasks.remove(property);
    d_ptr->m_values.remove(property);
}
