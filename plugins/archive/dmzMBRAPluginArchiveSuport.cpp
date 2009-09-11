#include "dmzMBRAPluginArchiveSuport.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleMap.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>

dmz::MBRAPluginArchiveSuport::MBRAPluginArchiveSuport (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      ArchiveObserverUtil (Info, local),
      _log (Info),
      _undo (Info),
      _map (0),
      _offsetX (0.5),
      _offsetY (0.5),
      _defaultAttrHandle (0),
      _storeObjects (False),
      _version (-1) {

   _init (local);
}


dmz::MBRAPluginArchiveSuport::~MBRAPluginArchiveSuport () {

}


// Plugin Interface
void
dmz::MBRAPluginArchiveSuport::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginArchiveSuport::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_map) { _map = QtModuleMap::cast (PluginPtr); }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_map && (_map == QtModuleMap::cast (PluginPtr))) { _map = 0; }
   }
}


// Object Observer Interface
void
dmz::MBRAPluginArchiveSuport::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (_storeObjects && _typeSet.contains_type (Type)) {

      _objects.add_handle (ObjectHandle);
   }
}

// Archive Observer Interface
void
dmz::MBRAPluginArchiveSuport::pre_process_archive (
      const Handle ArchiveHandle,
      const Int32 Version) {

   if (Version <= _version) {

      _objects.clear ();
      _storeObjects = True;
   }
}


void
dmz::MBRAPluginArchiveSuport::post_process_archive (
      const Handle ArchiveHandle,
      const Int32 Version) {

   if (_storeObjects) {

      ObjectModule *objMod = get_object_module ();

      if ((_objects.get_count () > 0) && objMod) {

         Float64 minx = 1.0e32, miny = 1.0e32, maxx = -1.0e32, maxy = -1.0e32;
         HandleContainerIterator it;
         Handle object (0);

         while (_objects.get_next (it, object)) {

            Vector pos;

            if (objMod->lookup_position (object, _defaultAttrHandle, pos)) {

               const Float64 XX = pos.get_x ();
               const Float64 YY = pos.get_z ();

               if (XX < minx) { minx = XX; }
               if (XX > maxx) { maxx = XX; }

               if (YY < miny) { miny = YY; }
               if (YY > maxy) { maxy = YY; }
            }
         }

         const Float64 OffsetX = minx + ((maxx - minx) * 0.5);
         const Float64 OffsetY = miny + ((maxy - miny) * 0.5);
         const Float64 Scale = is_zero64 (maxx - minx) ? 1.0 : 0.5 / (maxx - minx);

         it.reset ();

         while (_objects.get_next (it, object)) {

            Vector pos;

            if (objMod->lookup_position (object, _defaultAttrHandle, pos)) {

               pos.set_x ((pos.get_x () - OffsetX) * Scale);
               pos.set_z ((pos.get_z () - OffsetY) * Scale);
               objMod->store_position (object, _defaultAttrHandle, pos);
            }
         }

         if (_map) {

            _map->center_on (0.0, 0.0);
            _map->set_zoom (10.0);
         }

         _undo.reset ();
      }
   }

   _storeObjects = False;
}


void
dmz::MBRAPluginArchiveSuport::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   init_archive (local);

   _defaultAttrHandle = activate_default_object_attribute (ObjectCreateMask);

   _typeSet = config_to_object_type_set ("object-type-list", local, context);

   if (_typeSet.get_count () == 0) {

      _typeSet.add_object_type ("na_node", context);
   }
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginArchiveSuport (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginArchiveSuport (Info, local);
}

};
