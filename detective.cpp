#include "detective.h"

static inline void ltrim(std::string & s);
static inline void rtrim(std::string & s);
static inline void trim(std::string & s);
bool smaller_timeline(const timeline & a, const timeline & b);
bool earlier_match(const std::pair < int, int >&a, const std::pair < int, int >&b);

bool Detective::events_between_matchpoints(int i, const timeline & array1, const timeline & array2)
{
	// if there are non-matching events between 2 matchpoints on both timelines then they can never be fully merged
	matchpoints points = find_matchingpoints(array1, array2);
	assert(i < (int)points.size());
	int matchpoint_left_timeline1 = points.at(i).first;
	int matchpoint_left_timeline2 = points.at(i).second;
	int matchpoint_right_timeline1 = points.at(i + 1).first;
	int matchpoint_right_timeline2 = points.at(i + 1).second;

	bool events_between_matchpoints_timeline1 = matchpoint_right_timeline1 - matchpoint_left_timeline1 != 1;
	bool events_between_matchpoints_timeline2 = matchpoint_right_timeline2 - matchpoint_left_timeline2 != 1;
	if (events_between_matchpoints_timeline1 && events_between_matchpoints_timeline2) {
		return true;
	}
	return false;
}

bool Detective::conflicts_at_edge(const timeline & array1, const timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	int left_first = points.at(0).first;
	int left_second = points.at(0).second;
	int right_first = points.at(points.size() - 1).first;
	int right_second = points.at(points.size() - 1).second;

	if (left_first != 0 && left_second != 0) {
        // conflict left
		return true;
	}
	if (right_first != (int)array1.size() - 1 && right_second != (int)array2.size() - 1) {
		// conflict right
        return true;
	}
	return false;
}

// copies events from longer left side to the shorter timeline
void Detective::partialmerge_longer_left(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);
	int left_first = points.at(0).first;
	int left_second = points.at(0).second;

	if (left_first == 0 && left_second != 0) {
		// copy everything from 0 until matchpoint_left_timeline2 -1 to timeline1
		for (int i = 0; i < left_second; i++) {
			auto it = array1.begin();
			it = it + i;
			array1.insert(it, array2.at(i));
		}
	} else if (left_first != 0 && left_second == 0) {
		for (int i = 0; i < left_first; i++) {
			auto it = array2.begin();
			it = it + i;
			array2.insert(it, array1.at(i));
		}
	}
}

void Detective::partialmerge_longer_right(timeline & array1, timeline & array2) {
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);
	int right_first = points.at(points.size() - 1).first;
	int right_second = points.at(points.size() - 1).second;

	if ((right_first == (int)array1.size() - 1 && right_second != (int)array2.size() - 1)) {
		// array 2 has more info at the end
		for (size_t i = right_second + 1; i < array2.size(); i++) {
			auto it = array1.end();
			array1.insert(it, array2.at(i));
		}
	} else if ((right_first != (int)array1.size() - 1 && right_second == (int)array2.size() - 1)) {
		// array 1 has more info at the end
		for (size_t i = right_first + 1; i < array1.size(); i++) {
			auto it = array2.end();
			array2.insert(it, array1.at(i));
		}
	}
}

void Detective::partialmerge_info_inbetween(timeline & array1, timeline & array2) {
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);

	for (size_t i = 0; i < points.size() - 1; i++) {	// iterate over all neighbours
		points = find_matchingpoints(array1, array2); // the indexes of the points change during the loop because of insertion
        // first pair
		int left_first = points.at(i).first;
		int left_second = points.at(i).second;
        //second pair
		int right_first = points.at(i + 1).first;
		int right_second = points.at(i + 1).second;

		if ((right_first - left_first == 1 && right_second - left_second != 1)) { // more info on array2
			auto it = array1.begin() + right_first;
			for (int j = right_second - 1; j > left_second; j--) {	// copy everything between those points
				it = array1.insert(it, array2.at(j));
			}
		} else if ((right_second - left_second == 1 && right_first - left_first != 1)) { // more info on array1
			auto it = array2.begin() + right_second;
			for (int j = right_first - 1; j > left_first; j--) {
				it = array2.insert(it, array1.at(j));
			}
		}
	}
}

void Detective::write_timeline(const timeline & t, bool last) {
	if (t.size() == 0) {
		std::cout << "[]";
	} else {
		std::cout << "[";
		for (size_t j = 0; j < t.size() - 1; j++) {
			std::cout << "\"" << t.at(j) << "\"" << ",";
		}
		std::cout << "\"" << t.at(t.size() - 1) << "\"" << "]";
	}
	if (!last) {
		std::cout << ",";
	}
	std::cout << std::endl;
}

void Detective::load_json(const char *filename) {
	if (std::string(filename).find(".json") == std::string::npos) {
		std::cerr << "[Warning]: " << filename << "does not have .json extention. Ignoring file.\n";
	}
	std::ifstream file;
	file.open(filename);
	if (file.fail()) {
		std::cerr << "[Error] Could not open the provided file: " << filename << std::endl;
		exit(EXIT_FAILURE);
	}
	std::stringstream ss;
	ss << file.rdbuf();
	std::string s = ss.str();
	trim(s);
	if (s.size() == 0) {
		std::cerr << "[Error] " << filename << " appears to be empty" << std::endl;
		exit(EXIT_FAILURE);
	}
	s[0] = '{';
	s[s.size() - 1] = '}';

	int index = 0;
	const std::string replace = "[";
	std::string::size_type n = 0;
	while ((n = s.find(replace, n)) != std::string::npos) {
		const std::string t = "\"" + std::to_string(index) + "\":[";
		std::string insert_string = "\"" + std::to_string(index) + "\":";
		index++;
		s.insert(n, insert_string);
		n += t.size();
	}
	int number_of_timelines = index;
	try {
		JSON json = JSON::parse(s);
		for (int i = 0; i < number_of_timelines; i++) {
			timeline t = (timeline) json[std::to_string(i)];
			timelines.push_back(t);
		}
	}
	catch( ...) {
		std::cerr << "[Error] The formatting of " << filename << " does not seem to be correct" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Detective::write_json() {
	std::sort(timelines.begin(), timelines.end(), smaller_timeline);
	std::cout << "[" << std::endl;
	for (size_t i = 0; i < timelines.size() - 1; i++) {
		timeline t = timelines.at(i);
		write_timeline(t, false);
	}
	write_timeline(timelines.at(timelines.size() - 1), true);
	std::cout << "]" << std::endl;
}

Detective::Detective(const char *filename) {
	load_json(filename);
}

bool Detective::can_timelines_merge(timeline & array1, timeline & array2) {
	matchpoints points = find_matchingpoints(array1, array2);
	if (points.size() == 0) { return false; }
	if (points.size() == 1) {
		int matchpoint_timeline1 = points.at(0).first;
		int matchpoint_timeline2 = points.at(0).second;

		if(array1.size()==1 && array2.size()==1) { // e.g. ["cat"] and ["cat"]
			return true;
		}

		// three reasons why merge is not possible: 
		// both matches are at the same edge, there is no match at an edge or non-matching events overlap
		bool both_at_same_edge = (matchpoint_timeline1 == 0 && matchpoint_timeline2 == 0) || (matchpoint_timeline1 == (int)array1.size() - 1 && matchpoint_timeline2 == (int)array2.size() - 1);
		bool no_match_at_edge = (matchpoint_timeline1 != 0 && matchpoint_timeline1 != (int)array1.size() - 1 && matchpoint_timeline2 != 0 && matchpoint_timeline2 != (int)array2.size() - 1);
		bool overlap = (matchpoint_timeline1!=0 && matchpoint_timeline2!=0) || (matchpoint_timeline1!=(int)array1.size() - 1 && matchpoint_timeline2!=(int)array2.size() - 1);
        if (both_at_same_edge || no_match_at_edge || overlap) {
			return false;
		}
		return true;
	} 
	for (size_t i = 0; i < points.size() - 1; i++) {
		if (events_between_matchpoints(i, array1, array2)) {
			return false;
		}
	}
	if (conflicts_at_edge(array1, array2)) {
		return false;
	}
	return true;
}

bool Detective::is_partial_merge_possible(timeline & array1, timeline & array2) {
	matchpoints points = find_matchingpoints(array1, array2);
	if (points.size() == 0) { return false;}

	if(points.size()==1) {
		int first = points.at(0).first;
		int second = points.at(0).second;
		bool max_one_match_at_one_edge = (first == 0 && second != 0)
	    || (first != 0 && second == 0)
	    || (first != (int)array1.size() - 1 && second == (int)array2.size() - 1)
	    || (first == (int)array1.size() - 1 && second != (int)array2.size() - 1);
		return max_one_match_at_one_edge;
	}

	int left_first = points.at(0).first;
	int left_second = points.at(0).second;
	int right_first = points.at(points.size() - 1).first;
	int right_second = points.at(points.size() - 1).second;

	// partial merge is possible when 1 of the timeline starts/ends with a matching point while the other one has non-matching events before that
	bool max_one_match_at_one_edge = (left_first == 0 && left_second != 0)
	    || (left_first != 0 && left_second == 0)
	    || (right_first != (int)array1.size() - 1 && right_second == (int)array2.size() - 1)
	    || (right_first == (int)array1.size() - 1 && right_second != (int)array2.size() - 1);
	bool points_inbetween = (right_first - left_first == 1 && right_second - left_second != 1) || (right_first - left_first != 1 && right_second - left_second == 1);
	return (max_one_match_at_one_edge || points_inbetween);
}

void Detective::merge_timelines(timeline & array1, timeline & array2) {
	matchpoints points = find_matchingpoints(array1, array2);
	if (points.size() == 0) {
		return;
	}

	partialmerge_longer_left(array1, array2);

	partialmerge_longer_right(array1, array2);

	partialmerge_info_inbetween(array1, array2);
}

void Detective::detect() {
	if (timelines.size() == 0) {
		std::cerr << "No timelines where detected, check if the file that you provided isn't empty" << std::endl;
		exit(1);
	}
	if (timelines.size() == 1) {
		return;
	}
	std::sort(timelines.begin(), timelines.end(), smaller_timeline);
	bool changes = true;
	bool more_then_one_timeline = true;
	while (changes && more_then_one_timeline) {
		std::sort(timelines.begin(), timelines.end(), smaller_timeline);
		changes = false;
		size_t i = 1;
		while (i < timelines.size()) {
			size_t j = 0;
			bool merged;
			while (j < i) {
				if (can_timelines_merge(timelines.at(j), timelines.at(i))) {
					merge_timelines(timelines.at(j), timelines.at(i));
					changes = true;
					merged = true;

				} else if (is_partial_merge_possible(timelines.at(j), timelines.at(i))) {
					merge_timelines(timelines.at(j), timelines.at(i));
					changes = true;
				}
				j++;
			}
			if (merged) {	// if merges have taken place the duplicate can be deleted
				auto it = timelines.begin();
				it += i;
				timelines.erase(it);
				i--;	// since you erased a timeline at that position is now another timeline
			}
			i++;
			more_then_one_timeline = timelines.size();
		}
	}
}

matchpoints Detective::find_matchingpoints(timeline array1, timeline array2)
{
	// function returns an array containing 2 arrays, the first array contains the indexes of all matching points in array1, the second array contains the indexes of all matching points in array2
	matchpoints result;
	for (size_t index_array1 = 0; index_array1 < array1.size(); index_array1++) {
		for (size_t index_array2 = 0; index_array2 < array2.size(); index_array2++) {
			if (array1[index_array1] == array2[index_array2]) {
				std::pair < int, int >match;
				match.first = index_array1;
				match.second = index_array2;
				result.push_back(match);
			}
		}
	}
	std::sort(result.begin(), result.end(), earlier_match);
	if (result.size() == 0) {
        return result;

	} 
	return result;
}

static void ltrim(std::string & s) {// trim from start (in place)
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),[](unsigned char ch) {
		return !std::isspace(ch);
	}
	));
}

static void rtrim(std::string & s)
{				// trim from end (in place)
	s.erase(std::find_if(s.rbegin(), s.rend(),[](unsigned char ch) {
			     return !std::isspace(ch);
			     }
		).base(), s.end());
}

static void trim(std::string & s)
{
	ltrim(s);
	rtrim(s);
}				// trim from both ends (in place)

bool smaller_timeline(const timeline & a, const timeline & b){ return a.size() > b.size();}

bool earlier_match(const std::pair < int, int >&a, const std::pair < int, int >&b){ return a.first < b.first;}
