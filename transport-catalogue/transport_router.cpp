#include "transport_router.h"

#include <utility>
#include <iostream>

namespace transport_catalogue {

TransportRouter::TransportRouter(const TransportCatalogue& tc, RoutingSettings settings)
	: settings_(std::move(settings))
{
	BuildGraph(tc);
}

std::optional<RouteInfo> TransportRouter::BuildRoute(const std::string_view from, const std::string_view to) const {

	auto route = router_->BuildRoute(stops_name_to_id_.at(from), stops_name_to_id_.at(to));
	
	if (!route) {
		return std::nullopt;
	}

	RouteInfo result;
	result.items.reserve(route->edges.size());
	for (const auto& id : route->edges) {
		result.route_time += id_to_route_item_.at(id).time;
		result.items.push_back(id_to_route_item_.at(id));
	}
	return result;
}

void TransportRouter::BuildGraph(const TransportCatalogue& tc) {
	graph::DirectedWeightedGraph<double> graph(tc.GetStopsCount() * 2);
	graph::VertexId vertex_id {};

	// Рёбра Wait (между чентой и нечетной вершиной) A(bus) - A1(Wait)
	for (const auto& [stop_name, stop_ptr] : tc.GetAllStops()) {
		stops_name_to_id_.insert({ stop_name, vertex_id });
		auto edge_id = graph.AddEdge({ vertex_id, ++vertex_id , settings_.bus_wait_time * 1. });
		id_to_route_item_.insert({ edge_id,
			{
				ItemType::WAIT,
				settings_.bus_wait_time * 1.,
				stop_name
			}
		});
		++vertex_id;
	}

	// Рёбра Bus
	for (const auto& [bus_name, bus_ptr] : tc.GetAllBuses()) {
		// Строим ребра Bus вида: A1 -> B, A1 -> C, A1 -> D, B1 -> C, B1 -> D, C1 -> D
		for (int from = 0; from < bus_ptr->stops.size() - 1; ++from) {
			uint32_t distance = 0;
			int span_count = 1;
			for (int to = from + 1; to < bus_ptr->stops.size(); ++to) {
				distance += tc.GetDistance(bus_ptr->stops[to - 1], bus_ptr->stops[to]);
				double bus_time = distance / (settings_.bus_velocity * 1000.0 / 60.0);
				auto edge_id = graph.AddEdge({ stops_name_to_id_[bus_ptr->stops[from]->id] + 1 // Bus
					, stops_name_to_id_[bus_ptr->stops[to]->id] //Wait
					, bus_time
					});
				id_to_route_item_.insert({ edge_id,
					{
						ItemType::BUS,
						bus_time,
						bus_ptr->id,
						span_count
					}
				});
				++span_count;

				//if (!bus_ptr->is_roundtrip && bus_ptr->stops[to + 1] == bus_ptr->last_stop) {
				//	break; // На конечной выходим
				//}
				/**/
			}
		}
	}

	graph_ = std::move(graph);
	router_ = std::make_unique<graph::Router<double>>(graph_);
}

} // namespace transport_catalogue