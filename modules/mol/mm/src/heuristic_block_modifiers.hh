#ifndef HEURISTIC_BLOCK_MODIFIERS
#define HEURISTIC_BLOCK_MODIFIERS


#include <boost/shared_ptr.hpp>

#include <ost/message.hh>
#include <ost/mol/bond_handle.hh>
#include <ost/mol/residue_handle.hh>
#include <ost/mol/entity_handle.hh>
#include <ost/mol/atom_handle.hh>
#include <ost/mol/xcs_editor.hh>
#include <ost/geom/vec3.hh>
#include <ost/mol/mm/block_modifiers.hh>
#include <ost/mol/mm/buildingblock.hh>


namespace ost{ namespace mol{ namespace mm{

class HeuristicHydrogenConstructor;
class HeuristicTerminiConstructor;
class HeuristicBlockModifier;

typedef boost::shared_ptr<HeuristicHydrogenConstructor> HeuristicHydrogenConstructorPtr;
typedef boost::shared_ptr<HeuristicTerminiConstructor> HeuristicTerminiConstructorPtr;
typedef boost::shared_ptr<HeuristicBlockModifier> HeuristicBlockModifierPtr; 


class HeuristicHydrogenConstructor : public HydrogenConstructor{

public:

  HeuristicHydrogenConstructor(BuildingBlockPtr block);

  virtual void ApplyOnBuildingBlock(BuildingBlockPtr);

  virtual void ApplyOnResidue(ost::mol::ResidueHandle& res, ost::mol::XCSEditor& ed);

  virtual void OnSave(ost::io::BinaryDataSink& ds) { ds << *this; }

  virtual BlockModifierType GetBlockModifierType() { return HeuristicBlockModifiers; }
  
  template <typename DS>
  
  void Serialize(DS& ds){

    if(ds.IsSource()){
      int num_anchor_atoms = 0;
      int num_antecendents = 0;
      int num_hydrogens = 0;
      String loaded_string;

      ds & num_anchor_atoms;

      for(int i = 0; i < num_anchor_atoms; ++i){
        ds & num_antecendents;
        ds & num_hydrogens;

        ds & loaded_string;
        anchor_atom_names_.push_back(loaded_string);

        antecedent_names_.push_back(std::vector<String>());
        for(uint j = 0; j < num_antecendents; ++j){
          ds & loaded_string;
          antecedent_names_[i].push_back(loaded_string);
        }

        hydrogen_names_.push_back(std::vector<String>());
        for(uint j = 0; j < num_hydrogens; ++j){
          ds & loaded_string;
          hydrogen_names_[i].push_back(loaded_string);
        }
      }
    }
    else{
      int num_anchor_atoms = anchor_atom_names_.size();
      int num_antecendents = 0;
      int num_hydrogens = 0;

      ds & num_anchor_atoms;

      for(uint i = 0; i < num_anchor_atoms; ++i){

        num_antecendents = antecedent_names_[i].size();
        num_hydrogens = hydrogen_names_[i].size();
        ds & num_antecendents;
        ds & num_hydrogens;
        ds & anchor_atom_names_[i];


        for(std::vector<String>::iterator j = antecedent_names_[i].begin();
            j != antecedent_names_[i].end(); ++j){
          ds & *j;
        }

        for(std::vector<String>::iterator j = hydrogen_names_[i].begin();
            j != hydrogen_names_[i].end(); ++j){
          ds & *j;
        }
      }

    }
  }

private:
  std::vector<String> anchor_atom_names_;
  std::vector<std::vector<String> > antecedent_names_;
  std::vector<std::vector<String> > hydrogen_names_;
};


class HeuristicBlockModifier : public BlockModifier{

public:

  HeuristicBlockModifier() { }

  virtual void ApplyOnBuildingBlock(BuildingBlockPtr p);

  virtual void ApplyOnResidue(ost::mol::ResidueHandle& res, ost::mol::XCSEditor& ed);

  virtual BlockModifierType GetBlockModifierType() { return HeuristicBlockModifiers; }
  
  virtual void OnSave(ost::io::BinaryDataSink& ds) { ds << *this; }

  template <typename DS>
  
  void Serialize(DS& ds){
    
  }

};



}}}

#endif
