#include "kinematic_filter.h"

using namespace planner;

kinematic_filter::kinematic_filter()
{
//     left_joints.resize(kinematics.left_leg.getNrOfJoints());
//     right_joints.resize(kinematics.right_leg.getNrOfJoints());
//     leg_joints.resize(kinematics.RL_legs.getNrOfJoints());
//     SetToZero(left_joints);
//     SetToZero(right_joints);
//     SetToZero(leg_joints);
}

void kinematic_filter::setWorld_StanceFoot(const KDL::Frame& World_StanceFoot)
{
    this->StanceFoot_World=World_StanceFoot.Inverse();
    this->World_StanceFoot=World_StanceFoot;
}

void kinematic_filter::setLeftRightFoot(bool left)
{
    if (left)
    {
//         current_joints=left_joints;
        current_ik_solver=kinematics.lwr_legs.iksolver;
        current_chain_names=kinematics.lwr_legs.joint_names;
//         current_fk_solver=kinematics.fkLsolver;
    }
    else
    {
//         current_joints=right_joints;
        current_ik_solver=kinematics.rwl_legs.iksolver;
        current_chain_names=kinematics.rwl_legs.joint_names;
//         current_fk_solver=kinematics.fkRsolver;

    }
}

std::vector< std::string > kinematic_filter::getJointOrder()
{
    return current_chain_names;
}


bool kinematic_filter::filter(std::list<foot_with_joints> &data)
{
    jnt_pos_in=kinematics.lwr_legs.joints_value;
    
    for (auto single_step=data.begin();single_step!=data.end();)
    {
        auto StanceFoot_MovingFoot=StanceFoot_World*single_step->World_MovingFoot;
        single_step->World_StanceFoot=World_StanceFoot;
        if (!frame_is_reachable(StanceFoot_MovingFoot,single_step->joints))
            single_step=data.erase(single_step);
        else
            single_step++;
    }
    return true;
}

bool kinematic_filter::frame_is_reachable(const KDL::Frame& StanceFoot_MovingFoot, KDL::JntArray& jnt_pos)
{
    SetToZero(jnt_pos_in);
    KDL::JntArray jnt_pos_out(kinematics.rwl_legs.chain.getNrOfJoints());
    int ik_valid = current_ik_solver->CartToJnt(jnt_pos_in, StanceFoot_MovingFoot, jnt_pos_out);
    if (ik_valid>=0)
    {
        jnt_pos=jnt_pos_out;
        return true;
    }

    return false;
}
