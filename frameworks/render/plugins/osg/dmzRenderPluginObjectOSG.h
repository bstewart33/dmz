#ifndef DMZ_RENDER_PLUGIN_OBJECT_OSG_DOT_H
#define DMZ_RENDER_PLUGIN_OBJECT_OSG_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeDefinitionsObserver.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeResources.h>
#include <dmzRuntimeResourcesObserver.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <dmzTypesDeleteListTemplate.h>

#include <osg/Switch>

namespace dmz {

   class RenderModuleCoreOSG;

   class RenderPluginObjectOSG :
         public Plugin,
         public ResourcesObserver,
         public DefinitionsObserver,
         public ObjectObserverUtil {

      public:
         RenderPluginObjectOSG (const PluginInfo &Info, Config &local);
         ~RenderPluginObjectOSG ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Resources Observer Interface
         virtual void update_resource (
            const String &Name,
            const ResourcesUpdateTypeEnum Mode);

         // Definitions Observer Interface
         virtual void define_object_type (const ObjectType &Type);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void update_object_state (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Mask &Value,
            const Mask *PreviousValue);

         virtual void update_object_flag (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Boolean Value,
            const Boolean *PreviousValue);

         virtual void update_object_scale (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Vector &Value,
            const Vector *PreviousValue);

      protected:
         struct ModelStruct {

            osg::ref_ptr<osg::Node> model;
         };

         struct StateStruct {

            const unsigned int Place;
            const Mask State;

            StateStruct *next;

            StateStruct (const unsigned int ThePlace, const Mask TheState) :
               Place (ThePlace),
               State (TheState),
               next (0) {;}

            ~StateStruct () {;}
         };

         struct DefStruct {

            const Boolean Glyph;
            osg::ref_ptr<osg::Switch> model;
            StateStruct *stateMap;

            DefStruct (const Boolean IsGlyph) : Glyph (IsGlyph), stateMap (0) {

               model = new osg::Switch;
               model->setDataVariance (osg::Object::DYNAMIC);
            }

            ~DefStruct () { delete_list (stateMap); }
         };
 
         struct ObjectStruct {

            const DefStruct &Def;
            osg::ref_ptr<osg::Switch> model;

            ObjectStruct (DefStruct &TheDef) : Def (TheDef) {

               if (Def.model.valid ()) {

                  model = (osg::Switch *)Def.model->clone (osg::CopyOp::DEEP_COPY_NODES);
               }
            }
         };

         DefStruct *_lookup_def_struct (const ObjectType &Type);
         DefStruct *_create_def_struct (const ObjectType &Type);
         ModelStruct *_load_model (const String &FileName);
         void _add_models ();
         void _remove_models ();
         void _init (Config &local);

         Log _log;
         Definitions _defs;
         Resources _rc;

         RenderModuleCoreOSG *_core;

         HashTableStringTemplate<ModelStruct> _modelTable;
         HashTableHandleTemplate<DefStruct> _defTable;
         HashTableHandleTemplate<DefStruct> _typeTable;
         HashTableHandleTemplate<ObjectStruct> _objectTable;
         ObjectTypeSet _ignoreType;

         ModelStruct _noModel;
         UInt32 _cullMask;
         UInt32 _masterIsectMask;
         UInt32 _entityIsectMask;
         UInt32 _glyphIsectMask;

      private:
         RenderPluginObjectOSG ();
         RenderPluginObjectOSG (const RenderPluginObjectOSG &);
         RenderPluginObjectOSG &operator= (const RenderPluginObjectOSG &);

   };
};

#endif // DMZ_RENDER_PLUGIN_OBJECT_OSG_DOT_H
