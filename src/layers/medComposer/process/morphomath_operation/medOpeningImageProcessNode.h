#pragma once
/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2020. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medAbstractOpeningImageProcess.h>
#include <medMorphomathOperationProcessNode.h>

#include <medComposerExport.h>

class medOpeningImageProcessNodePrivate;

class MEDCOMPOSER_EXPORT medOpeningImageProcessNode : public medMorphomathOperationProcessNode<medAbstractOpeningImageProcess>
{
public:
     medOpeningImageProcessNode();
};


