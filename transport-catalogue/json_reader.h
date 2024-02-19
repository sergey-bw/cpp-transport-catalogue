#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue::io {

class JsonReader {
public:
	JsonReader(json::Document document);

	JsonReader(std::istream& input);

	void FillCatalogue(TransportCatalogue& tc) const;

	void StatRequest(RequestHandler& request_handler, std::ostream& output) const;

	renderer::RenderSettings GetRendererSettings() const;

private:
	json::Document document_;

	const json::Node& JsonGetBaseRequest() const;

	const json::Node& JsonGetStatRequest() const;

	const json::Node& JsonGetRenderSettings() const;

	void FillCatalogueStop(TransportCatalogue& tc, const json::Node& stop_node) const;

	void FillCatalogueDistances(TransportCatalogue& tc, const std::string_view from, const json::Node& distances_node) const;

	void FillCatalogueBus(TransportCatalogue& tc, const json::Node& bus_node) const;

	svg::Color JsonParseColor(const json::Node& color_node) const;

	bool ISJsonRenderSettings() const;
};

}  // namespace transport_catalogue::io