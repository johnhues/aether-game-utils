//------------------------------------------------------------------------------
// NetTest.cpp
// Copyright (c) John Hughes on 2/9/24. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// ae::NetObjectServer
//------------------------------------------------------------------------------
TEST_CASE( "Create and destroy NetObjects", "[ae::NetObjectServer]" )
{
	ae::NetObjectServer server;
	ae::NetObject* netObj = server.CreateNetObject();
	REQUIRE( netObj != nullptr );
	server.DestroyNetObject( netObj );
	server.UpdateSendData();
}
