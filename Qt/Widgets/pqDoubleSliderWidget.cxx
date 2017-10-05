/*=========================================================================

  Program:   ParaView
  Module:    pqDoubleSliderWidget.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "pqDoubleSliderWidget.h"

#include "pqLineEdit.h"

#include "vtkPVConfig.h"

// Qt includes
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QSlider>

pqDoubleSliderWidget::pqDoubleSliderWidget(QWidget* parent)
  : QWidget(parent)
{
  this->BlockUpdate = false;
  this->Value = 0;
  this->InteractingWithSlider = false;
  this->DeferredValueEdited = false;

  QHBoxLayout* l = new QHBoxLayout(this);
  l->setMargin(0);
  this->Slider = new QSlider(Qt::Horizontal, this);
  this->Slider->setRange(0, 100);
  l->addWidget(this->Slider);
  this->Slider->setObjectName("Slider");
  this->LineEdit = new pqLineEdit(this);
  l->addWidget(this->LineEdit);
  this->LineEdit->setObjectName("LineEdit");
  this->LineEdit->setValidator(new QDoubleValidator(this->LineEdit));
  this->LineEdit->setTextAndResetCursor(QString().setNum(this->Value));

  QObject::connect(this->Slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
  QObject::connect(
    this->LineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
  QObject::connect(
    this->LineEdit, SIGNAL(textChangedAndEditingFinished()), this, SLOT(editingFinished()));

  // let's avoid firing `valueChanged` events until the user has released the
  // slider.
  this->connect(this->Slider, SIGNAL(sliderPressed()), SLOT(sliderPressed()));
  this->connect(this->Slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
}

//-----------------------------------------------------------------------------
pqDoubleSliderWidget::~pqDoubleSliderWidget()
{
}

//-----------------------------------------------------------------------------
double pqDoubleSliderWidget::value() const
{
  return this->Value;
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::setValue(double val)
{
  if (this->Value == val)
  {
    return;
  }

  this->Value = val;

  if (!this->BlockUpdate)
  {
    // set the slider
    this->updateSlider();

    // set the text
    this->BlockUpdate = true;
    this->LineEdit->setTextAndResetCursor(
      QString().setNum(val, 'g', DEFAULT_DOUBLE_PRECISION_VALUE));
    this->BlockUpdate = false;
  }

  emit this->valueChanged(this->Value);
}

//-----------------------------------------------------------------------------
int pqDoubleSliderWidget::valueToSliderPos(double val)
{
  return val;
}

//-----------------------------------------------------------------------------
double pqDoubleSliderWidget::sliderPosToValue(int pos)
{
  return pos;
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::sliderChanged(int val)
{
  if (!this->BlockUpdate)
  {
    double v = this->sliderPosToValue(val);
    this->BlockUpdate = true;
    this->LineEdit->setTextAndResetCursor(QString().setNum(v));
    this->setValue(v);
    this->emitValueEdited();
    this->BlockUpdate = false;
  }
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::textChanged(const QString& text)
{
  if (!this->BlockUpdate)
  {
    double val = text.toDouble();
    this->BlockUpdate = true;
    int sliderVal = this->valueToSliderPos(val);
    this->Slider->setValue(sliderVal);
    this->setValue(val);
    this->BlockUpdate = false;
  }
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::editingFinished()
{
  this->emitValueEdited();
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::setValidator(QDoubleValidator* validator)
{
  validator->setParent(this->LineEdit);
  this->LineEdit->setValidator(validator);
}

//-----------------------------------------------------------------------------
const QDoubleValidator* pqDoubleSliderWidget::validator() const
{
  return qobject_cast<const QDoubleValidator*>(this->LineEdit->validator());
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::setSliderRange(int min, int max)
{
  this->Slider->setRange(min, max);
  this->updateSlider();
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::updateSlider()
{
  this->Slider->blockSignals(true);
  int v = valueToSliderPos(this->Value);
  this->Slider->setValue(v);
  this->Slider->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::sliderPressed()
{
  this->InteractingWithSlider = true;
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::sliderReleased()
{
  this->InteractingWithSlider = false;
  this->emitIfDeferredValueEdited();
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::emitValueEdited()
{
  if (this->InteractingWithSlider == false)
  {
    emit this->valueEdited(this->Value);
  }
  else
  {
    this->DeferredValueEdited = true;
  }
}

//-----------------------------------------------------------------------------
void pqDoubleSliderWidget::emitIfDeferredValueEdited()
{
  if (this->DeferredValueEdited)
  {
    this->DeferredValueEdited = false;
    this->emitValueEdited();
  }
}