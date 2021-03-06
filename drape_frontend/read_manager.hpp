#pragma once

#include "drape_frontend/custom_symbol.hpp"
#include "drape_frontend/engine_context.hpp"
#include "drape_frontend/read_mwm_task.hpp"
#include "drape_frontend/tile_info.hpp"
#include "drape_frontend/tile_utils.hpp"

#include "geometry/screenbase.hpp"

#include "drape/object_pool.hpp"
#include "drape/pointers.hpp"
#include "drape/texture_manager.hpp"

#include "base/thread_pool.hpp"

#include "std/atomic.hpp"
#include "std/mutex.hpp"
#include "std/set.hpp"
#include "std/shared_ptr.hpp"
#include "std/target_os.hpp"

namespace df
{

class MapDataProvider;
class CoverageUpdateDescriptor;

uint8_t constexpr kReadingThreadsCount = 2;

class ReadManager
{
public:
  ReadManager(ref_ptr<ThreadsCommutator> commutator, MapDataProvider & model,
              bool allow3dBuildings, bool trafficEnabled);

  void UpdateCoverage(ScreenBase const & screen, bool have3dBuildings, bool forceUpdate,
                      TTilesCollection const & tiles, ref_ptr<dp::TextureManager> texMng);
  void Invalidate(TTilesCollection const & keyStorage);
  void InvalidateAll();
  void Stop();

  bool CheckTileKey(TileKey const & tileKey) const;
  void Allow3dBuildings(bool allow3dBuildings);

  void SetTrafficEnabled(bool trafficEnabled);

  void UpdateCustomSymbols(CustomSymbols const & symbols);
  void RemoveCustomSymbols(MwmSet::MwmId const & mwmId, std::vector<FeatureID> & leftoverIds);
  void RemoveAllCustomSymbols();

private:
  void OnTaskFinished(threads::IRoutine * task);
  bool MustDropAllTiles(ScreenBase const & screen) const;

  void PushTaskBackForTileKey(TileKey const & tileKey, ref_ptr<dp::TextureManager> texMng);

  ref_ptr<ThreadsCommutator> m_commutator;

  MapDataProvider & m_model;

  drape_ptr<threads::ThreadPool> m_pool;

  ScreenBase m_currentViewport;
  bool m_have3dBuildings;
  bool m_allow3dBuildings;
  bool m_trafficEnabled;
  bool m_modeChanged;

  struct LessByTileInfo
  {
    bool operator ()(shared_ptr<TileInfo> const & l, shared_ptr<TileInfo> const & r) const
    {
      return *l < *r;
    }
  };

  using TTileSet = set<shared_ptr<TileInfo>, LessByTileInfo>;
  TTileSet m_tileInfos;

  ObjectPool<ReadMWMTask, ReadMWMTaskFactory> myPool;

  int m_counter;
  mutex m_finishedTilesMutex;
  uint64_t m_generationCounter;

  using TTileInfoCollection = buffer_vector<shared_ptr<TileInfo>, 8>;
  TTilesCollection m_activeTiles;

  CustomSymbolsContextPtr m_customSymbolsContext;

  void CancelTileInfo(shared_ptr<TileInfo> const & tileToCancel);
  void ClearTileInfo(shared_ptr<TileInfo> const & tileToClear);
  void IncreaseCounter(int value);
  void CheckFinishedTiles(TTileInfoCollection const & requestedTiles);
};

} // namespace df
