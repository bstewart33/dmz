#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include "dmzQtPluginRenderPickCanvas.h"
#include <dmzRenderModulePickConvert.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeConfigToVector.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>
#include <QtGui/QtGui>


dmz::QtPluginRenderPickCanvas::QtPluginRenderPickCanvas (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      RenderPickUtil (Info, local),
      _log (Info),
      _canvasModule (0),
      _canvasModuleName (),
      _objectModule (0),
      _objectModuleName (),
      _discoverPickConvert (False),
      _pickConvertModule (0),
      _pickConvertModuleName () {

   // Initialize array
   _vectorOrder[0] = VectorComponentX;
   _vectorOrder[1] = VectorComponentY;
   _vectorOrder[2] = VectorComponentZ;

   _init (local);
}


dmz::QtPluginRenderPickCanvas::~QtPluginRenderPickCanvas () {

}


// Plugin Interface
void
dmz::QtPluginRenderPickCanvas::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvasModule) {

         _canvasModule = QtModuleCanvas::cast (PluginPtr, _canvasModuleName);
      }

      if (!_objectModule) {

         _objectModule = ObjectModule::cast (PluginPtr, _objectModuleName);
      }

      if (_discoverPickConvert && !_pickConvertModule) {

         _pickConvertModule = RenderModulePickConvert::cast (PluginPtr, _pickConvertModuleName);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvasModule && (_canvasModule == QtModuleCanvas::cast (PluginPtr))) {

         _canvasModule = 0;
      }

      if (_objectModule && (_objectModule == ObjectModule::cast (PluginPtr))) {

         _objectModule = 0;
      }

      if (_pickConvertModule && (_pickConvertModule == RenderModulePickConvert::cast (PluginPtr))) {

         _pickConvertModule = 0;
      }
   }
}


// RenderPick Interface
dmz::Boolean
dmz::QtPluginRenderPickCanvas::screen_to_world (
      const Int32 ScreenPosX,
      const Int32 ScreenPosY,
      Vector &worldPosition,
      Vector &normal,
      Handle &objectHandle) {

   Boolean retVal (False);

   if (_canvasModule) {

      QGraphicsView *view (_canvasModule->get_view ());

      if (view) {

         QPoint sourcePoint (view->mapFromGlobal (QPoint (ScreenPosX, ScreenPosY)));

         retVal = source_to_world (
            sourcePoint.x (),
            sourcePoint.y (),
            worldPosition,
            normal,
            objectHandle);
      }
   }

   return retVal;
}


dmz::Boolean
dmz::QtPluginRenderPickCanvas::world_to_screen (
      const Vector &WorldPosition,
      Int32 &screenPosX,
      Int32 &screenPosY) {

   Boolean retVal (False);

   if (_canvasModule) {

      Int32 sourceX;
      Int32 sourceY;

      if (world_to_source (WorldPosition, sourceX, sourceY)) {

         QGraphicsView *view (_canvasModule->get_view ());

         if (view) {

            QPoint screenPoint (view->mapToGlobal (QPoint (sourceX, sourceY)));

            screenPosX = screenPoint.x ();
            screenPosY = screenPoint.y ();

            retVal = True;
         }
      }
   }

   return retVal;
}


dmz::Boolean
dmz::QtPluginRenderPickCanvas::source_to_world (
      const Int32 SourcePosX,
      const Int32 SourcePosY,
      Vector &worldPosition,
      Vector &normal,
      Handle &objectHandle) {

   Boolean retVal (False);

   if (_canvasModule) {

      QGraphicsView *view (_canvasModule->get_view ());

      if (view) {
         
         QPoint sourcePoint (SourcePosX, SourcePosY);
         
         if (_pickConvertModule) {

            _pickConvertModule->source_to_world (
               SourcePosX, SourcePosY, worldPosition, normal, objectHandle);
         }
         else {

            QPointF worldPoint (view->mapToScene (sourcePoint));

            worldPosition.set (_vectorOrder[0], worldPoint.x ());
            worldPosition.set (_vectorOrder[1], worldPoint.y ());
            worldPosition.set (_vectorOrder[2], 0.0);
            normal.set_xyz (0.0, 0.0, 0.0);
            normal.set (_vectorOrder[2], 1.0);
         }
         
         objectHandle = _get_object_handle (sourcePoint);

         retVal = True;
      }
   }

   return retVal;
}


dmz::Boolean
dmz::QtPluginRenderPickCanvas::world_to_source (
      const Vector &WorldPosition,
      Int32 &sourcePosX,
      Int32 &sourcePosY) {

   Boolean retVal (False);

   if (_canvasModule) {

      QGraphicsView *view (_canvasModule->get_view ());

      if (view) {
         
         if (_pickConvertModule) {
            
            _pickConvertModule->world_to_source (WorldPosition, sourcePosX, sourcePosY);
         }
         else {
            
            QPointF worldPoint (
               WorldPosition.get (_vectorOrder[0]),
               WorldPosition.get (_vectorOrder[1]));

            QPoint sourcePoint (view->mapFromScene (worldPoint));

            sourcePosX = sourcePoint.x ();
            sourcePosY = sourcePoint.y ();
         }

         retVal = True;
      }
   }

   return retVal;
}


dmz::Handle
dmz::QtPluginRenderPickCanvas::_get_object_handle (const QPoint &ScreenPos) {

   Handle objectHandle (0);

   if (_canvasModule) {

      QGraphicsView *view (_canvasModule->get_view ());

      if (view) {

         QList<QGraphicsItem *>itemList (view->items (ScreenPos));

         if (itemList.count ()) {

            foreach (QGraphicsItem *item, itemList) {

               if (item->isVisible ()) {

                  bool ok (false);

                  qlonglong data (
                     item->data (QtCanvasObjectHandleIndex).toULongLong (&ok));

                  if (ok && !objectHandle && data) {

                     objectHandle = data;
                  }

                  if (objectHandle) { break; }
               }
            }
         }
      }
   }

   return objectHandle;
}


void
dmz::QtPluginRenderPickCanvas::_init (Config &local) {

   _canvasModuleName = config_to_string ("module.canvas.name", local);
   _objectModuleName = config_to_string ("module.object.name", local);
   _pickConvertModuleName = config_to_string ("module.pickConvert.name", local);
   _discoverPickConvert = config_to_boolean ("discover.pickConvert", local, _discoverPickConvert);

   _vectorOrder[0] = config_to_vector_component ("order.x", local, _vectorOrder [0]);
   _vectorOrder[1] = config_to_vector_component ("order.y", local, _vectorOrder [1]);
   _vectorOrder[2] = config_to_vector_component ("order.z", local, _vectorOrder [2]);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzQtPluginRenderPickCanvas (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::QtPluginRenderPickCanvas (Info, local);
}

};
