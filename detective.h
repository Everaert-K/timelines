#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>       
#include <sstream> 
#include <algorithm> 
#include <cctype>
#include <locale>

using JSON = nlohmann::json;
using timeline = std::vector<std::string>;
using matchpoints = std::vector<std::pair<int, int> >;

class Detective {
    std::vector<timeline> timelines;
    void load_json(const char* filename);
    void write_timeline(const timeline& t, bool last);
    matchpoints find_matchingpoints(timeline array1, timeline array2);
    void merge_timelines(timeline& array1, timeline& array2);
    void partially_merge_timelines(timeline& array1, timeline& array2);
    void partialmerge_longer_left(timeline& array1, timeline& array2);
    bool can_timelines_merge(timeline& array1, timeline& array2);
    bool is_partial_merge_possible(timeline& array1, timeline& array2);
    void partialmerge_info_inbetween(timeline& array1, timeline& array2);
    bool unwanted_events_between_matchpoints_on_both_timelines(int i, const timeline& array1, const timeline& array2);
    bool conflicts_at_edge(const timeline& array1, const timeline& array2);
    void partialmerge_longer_right(timeline& array1, timeline& array2);
public:
    Detective(const char* filename);
    void detect();
    void write_json();
};


