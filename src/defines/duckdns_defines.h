//---------------------------------------------------
// myFP2ESP8266 DUCKDNS SERVICE
// Copyright Robert Brown 2020-2025.
// All Rights Reserved.
//---------------------------------------------------
// Cannot use DuckDNS with ACCESSPOINT mode
// [STATIONMODE only]

// if using DuckDNS you need to set 
//     duckdnsdomain
//     duckdnstoken

char duckdnsdomain[48] = "bob.duckdns.org";
char duckdnstoken[48] = "1b0365d5-4831-32fe-c4f1-3d912b9af13d";

// NOTE
// These values are only used to generate default 
// duckdns settings in the ControllerData config 
// files.
// Runtime values come from ControllerData
