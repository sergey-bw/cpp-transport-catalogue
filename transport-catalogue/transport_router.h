#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <vector>
#include <string>

namespace transport_catalogue {

struct RoutingSettings {
	int bus_wait_time = 0;
	int bus_velocity = 0;
};

class TransportRouter {
public:
	TransportRouter() = delete;

	TransportRouter(const TransportCatalogue& tc, RoutingSettings settings);

	void BuildGraph(const TransportCatalogue& tc);

	std::optional<RouteInfo> BuildRoute(const std::string_view from, const std::string_view to) const;

private:
	RoutingSettings settings_;
	graph::DirectedWeightedGraph<double> graph_;
	std::unique_ptr<graph::Router<double>> router_ = nullptr;
	std::unordered_map<std::string_view, graph::VertexId> stops_name_to_id_;
	std::unordered_map<graph::EdgeId, RouteItem> id_to_route_item_;
};

} // namespace transport_catalogue