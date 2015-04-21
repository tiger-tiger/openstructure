#ifndef OST_SIMULATION_HH
#define OST_SIMULATION_HH

#include <vector>
#include <map>

#include <ost/mol/mol.hh>
#include <ost/mol/mm/system_creator.hh>
#include <ost/mol/mm/topology.hh>
#include <ost/mol/mm/topology_creator.hh>
#include <ost/mol/mm/settings.hh>
#include <ost/mol/mm/modeller.hh>
#include <ost/geom/vec3.hh>
#include <ost/mol/mm/observer.hh>
#include <ost/mol/mm/state_extractor.hh>
#include <ost/mol/mm/steep.hh>

#include <time.h>

namespace OpenMM{
  class Integrator;
  class Context;//hacky way of telling the Context and Integrator 
                //are around.
                //will be included in source file to avoid
                //dependencies on external libraries
}

namespace ost { namespace mol{ namespace mm{

class Simulation;

typedef boost::shared_ptr<OpenMM::Integrator> IntegratorPtr;
typedef boost::shared_ptr<OpenMM::Context> ContextPtr;
typedef boost::shared_ptr<Simulation> SimulationPtr;

class Simulation {

public:

  Simulation(const ost::mol::EntityHandle& handle, 
             const SettingsPtr settings);

  Simulation(const ost::mol::mm::TopologyPtr top,
             const ost::mol::EntityHandle& handle,
             const SettingsPtr settings);

  void Save(const String& filename);

  static SimulationPtr Load(const String& filename, SettingsPtr settings);

  ost::mol::EntityHandle GetEntity() { return ent_; }

  geom::Vec3List GetPositions(bool enforce_periodic_box = false, bool in_angstrom = true);

  geom::Vec3List GetVelocities();

  geom::Vec3List GetForces();

  void SetPositions(const geom::Vec3List& positions, bool in_angstrom = true);

  void SetVelocities(geom::Vec3List& velocities);

  void UpdatePositions(bool enforce_periodic_box = false);

  bool ApplySD(Real tolerance = 100, int max_iterations = 1000);

  void ApplyLBFGS(Real tolerance = 1, int max_iterations = 1000);

  Real GetEnergy();

  Real GetPotentialEnergy();

  Real GetKineticEnergy();

  TopologyPtr GetTopology() { return top_; }

  void Steps(int steps);

  void Register(ObserverPtr o);

  void ResetHarmonicBond(uint index, Real bond_length, Real force_constant);

  void ResetHarmonicAngle(uint index, Real angle, Real force_constant);

  void ResetUreyBradleyAngle(uint index, Real angle, Real angle_force_constant, Real bond_length, Real bond_force_constant);

  void ResetPeriodicDihedral(uint index, int multiplicity, Real phase, Real force_constant);

  void ResetPeriodicImproper(uint index, int multiplicity, Real phase, Real force_constant);

  void ResetHarmonicImproper(uint index, Real phase, Real force_constant);

  void ResetLJPair(uint index, Real sigma, Real epsilon);

  void ResetDistanceConstraint(uint index, Real constraint_length);

  void ResetHarmonicPositionRestraint(uint index, const geom::Vec3& ref_position, Real k,
                                      Real x_scale = 1.0, Real y_scale = 1.0, Real z_scale = 1.0);

  void ResetHarmonicDistanceRestraint(uint index, Real length, Real force_constant);

  void ResetLJ(uint index, Real sigma, Real epsilon);

  void ResetGBSA(uint index, Real radius, Real scaling);

  void ResetCharge(uint index, Real charge);

  void ResetMass(uint index, Real mass);

  void AddPositionConstraint(uint index);

  void AddPositionConstraints(const std::vector<uint>& index);

  void ResetPositionConstraints();

  geom::Vec3 GetPeriodicBoxExtents();

  void SetPeriodicBoxExtents(geom::Vec3& vec);

private:

  Simulation() { } //hidden constructor... 

  void Init(const ost::mol::mm::TopologyPtr top, 
            const SettingsPtr settings);

  int TimeToNextNotification();

  SystemPtr system_;
  IntegratorPtr integrator_;
  ContextPtr context_;
  TopologyPtr top_;
  std::vector<ObserverPtr> observers_;
  std::vector<int> time_to_notify_;
  std::map<FuncType,uint> system_force_mapper_;
  ost::mol::EntityHandle ent_;
};

}}} //ns

#endif
