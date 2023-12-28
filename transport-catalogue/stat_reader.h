#pragma once

#include <iostream>
#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const ctlg::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);
                       