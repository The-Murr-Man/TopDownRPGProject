// Copyright Kyle Murray


#include "UI/Widget/RPGUserWidget.h"

void URPGUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
