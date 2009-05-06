/* $Id$ */

/** @file ai_tunnel.cpp Implementation of AITunnel. */

#include "ai_tunnel.hpp"
#include "ai_rail.hpp"
#include "../ai_instance.hpp"
#include "../../tunnel_map.h"
#include "../../command_func.h"
#include "../../tunnelbridge.h"
#include "../../road_func.h"

/* static */ bool AITunnel::IsTunnelTile(TileIndex tile)
{
	if (!::IsValidTile(tile)) return false;
	return ::IsTunnelTile(tile);
}

/* static */ TileIndex AITunnel::GetOtherTunnelEnd(TileIndex tile)
{
	if (!::IsValidTile(tile)) return INVALID_TILE;

	/* If it's a tunnel alread, take the easy way out! */
	if (IsTunnelTile(tile)) return ::GetOtherTunnelEnd(tile);

	uint start_z;
	Slope start_tileh = ::GetTileSlope(tile, &start_z);
	DiagDirection direction = ::GetInclinedSlopeDirection(start_tileh);
	if (direction == INVALID_DIAGDIR) return INVALID_TILE;

	TileIndexDiff delta = ::TileOffsByDiagDir(direction);
	uint end_z;
	do {
		tile += delta;
		if (!::IsValidTile(tile)) return INVALID_TILE;

		::GetTileSlope(tile, &end_z);
	} while (start_z != end_z);

	return tile;
}

static void _DoCommandReturnBuildTunnel2(class AIInstance *instance)
{
	if (!AITunnel::_BuildTunnelRoad2()) {
		AIInstance::DoCommandReturn(instance);
		return;
	}

	/* This can never happen, as in test-mode this callback is never executed,
	 *  and in execute-mode, the other callback is called. */
	NOT_REACHED();
}

static void _DoCommandReturnBuildTunnel1(class AIInstance *instance)
{
	if (!AITunnel::_BuildTunnelRoad1()) {
		AIInstance::DoCommandReturn(instance);
		return;
	}

	/* This can never happen, as in test-mode this callback is never executed,
	 *  and in execute-mode, the other callback is called. */
	NOT_REACHED();
}

/* static */ bool AITunnel::BuildTunnel(AIVehicle::VehicleType vehicle_type, TileIndex start)
{
	EnforcePrecondition(false, ::IsValidTile(start));
	EnforcePrecondition(false, vehicle_type == AIVehicle::VT_RAIL || vehicle_type == AIVehicle::VT_ROAD);
	EnforcePrecondition(false, vehicle_type != AIVehicle::VT_RAIL || AIRail::IsRailTypeAvailable(AIRail::GetCurrentRailType()));

	uint type = 0;
	if (vehicle_type == AIVehicle::VT_ROAD) {
		type |= (TRANSPORT_ROAD << 9);
		type |= ::RoadTypeToRoadTypes((::RoadType)AIObject::GetRoadType());
	} else {
		type |= (TRANSPORT_RAIL << 9);
		type |= AIRail::GetCurrentRailType();
	}

	/* For rail we do nothing special */
	if (vehicle_type == AIVehicle::VT_RAIL) {
		return AIObject::DoCommand(start, type, 0, CMD_BUILD_TUNNEL);
	}

	AIObject::SetCallbackVariable(0, start);
	return AIObject::DoCommand(start, type, 0, CMD_BUILD_TUNNEL, NULL, &_DoCommandReturnBuildTunnel1);
}

/* static */ bool AITunnel::_BuildTunnelRoad1()
{
	/* Build the piece of road on the 'start' side of the tunnel */
	TileIndex end = AIObject::GetCallbackVariable(0);
	TileIndex start = AITunnel::GetOtherTunnelEnd(end);

	DiagDirection dir_1 = (DiagDirection)((::TileX(start) == ::TileX(end)) ? (::TileY(start) < ::TileY(end) ? DIAGDIR_NW : DIAGDIR_SE) : (::TileX(start) < ::TileX(end) ? DIAGDIR_NE : DIAGDIR_SW));
	DiagDirection dir_2 = ::ReverseDiagDir(dir_1);

	return AIObject::DoCommand(start + ::TileOffsByDiagDir(dir_1), ::DiagDirToRoadBits(dir_2) | (AIObject::GetRoadType() << 4), 0, CMD_BUILD_ROAD, NULL, &_DoCommandReturnBuildTunnel2);
}

/* static */ bool AITunnel::_BuildTunnelRoad2()
{
	/* Build the piece of road on the 'end' side of the tunnel */
	TileIndex end = AIObject::GetCallbackVariable(0);
	TileIndex start = AITunnel::GetOtherTunnelEnd(end);

	DiagDirection dir_1 = (DiagDirection)((::TileX(start) == ::TileX(end)) ? (::TileY(start) < ::TileY(end) ? DIAGDIR_NW : DIAGDIR_SE) : (::TileX(start) < ::TileX(end) ? DIAGDIR_NE : DIAGDIR_SW));
	DiagDirection dir_2 = ::ReverseDiagDir(dir_1);

	return AIObject::DoCommand(end + ::TileOffsByDiagDir(dir_2), ::DiagDirToRoadBits(dir_1) | (AIObject::GetRoadType() << 4), 0, CMD_BUILD_ROAD);
}

/* static */ bool AITunnel::RemoveTunnel(TileIndex tile)
{
	EnforcePrecondition(false, IsTunnelTile(tile));

	return AIObject::DoCommand(tile, 0, 0, CMD_LANDSCAPE_CLEAR);
}
