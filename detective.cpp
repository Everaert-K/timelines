#include "detective.h"

static inline void ltrim(std::string & s);
static inline void rtrim(std::string & s);
static inline void trim(std::string & s);
bool smaller_timeline(const timeline & a, const timeline & b);
bool earlier_match(const std::pair < int, int >&a, const std::pair < int, int >&b);

bool Detective::unwanted_events_between_matchpoints_on_both_timelines(int i, const timeline & array1, const timeline & array2)
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
		std::cerr << "[log] since there are non-matching points between the matching points, these 2 timelines can never be merged." << std::endl;
		return true;
	}
	return false;
}

bool Detective::conflicts_at_edge(const timeline & array1, const timeline & array2)
{
	// if they both aren't at the left edge and none of the 2 is neighbouring a matching point on its left side this can never be the occassion for the first matchpoint hence the i!=0
	matchpoints points = find_matchingpoints(array1, array2);
	int matchpoint_left_timeline1 = points.at(0).first;
	int matchpoint_left_timeline2 = points.at(0).second;
	int matchpoint_right_timeline1 = points.at(points.size() - 1).first;
	int matchpoint_right_timeline2 = points.at(points.size() - 1).second;
	bool result1 = false;
	if (matchpoint_left_timeline1 != 0 && matchpoint_left_timeline2 != 0) {
		result1 = true;
		std::cerr << "[log] There are conflicts at the left edge" << std::endl;
	}
	bool result2 = false;	// do the same as last rule but for the right side
	if (matchpoint_right_timeline1 != (int)array1.size() - 1 && matchpoint_right_timeline2 != (int)array2.size() - 1) {
		result2 = true;
		std::cerr << "[log] There are conflicts at the right edge" << std::endl;
	}
	return result1 || result2;
}

void Detective::partialmerge_longer_left(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);
	int matchpoint_left_timeline1 = points.at(0).first;
	int matchpoint_left_timeline2 = points.at(0).second;
	if (matchpoint_left_timeline1 == 0 && matchpoint_left_timeline2 != 0) {
		// copy everything from 0 until matchpoint_left_timeline2 -1 to timeline1
		std::cerr << "[log] (Partial) Merging: The second timeline has more info at the start, so let's copy this" << std::endl;
		for (int i = 0; i < matchpoint_left_timeline2; i++) {
			auto it = array1.begin();
			it = it + i;
			array1.insert(it, array2.at(i));
		}
	} else if (matchpoint_left_timeline1 != 0 && matchpoint_left_timeline2 == 0) {
		std::cerr << "[log] (Partial) Merging: The first timeline has more info at the start, so let's copy this" << std::endl;
		for (int i = 0; i < matchpoint_left_timeline1; i++) {
			auto it = array2.begin();
			it = it + i;
			array2.insert(it, array1.at(i));
		}
	}
}

void Detective::partialmerge_longer_right(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);
	int matchpoint_right_timeline1 = points.at(points.size() - 1).first;
	int matchpoint_right_timeline2 = points.at(points.size() - 1).second;
	if ((matchpoint_right_timeline1 == (int)array1.size() - 1 && matchpoint_right_timeline2 != (int)array2.size() - 1)) {
		std::cerr << "[log] (Partial) Merging: The second timeline has more info at the end, so let's copy this" << std::endl;
		for (size_t i = matchpoint_right_timeline2 + 1; i < array2.size(); i++) {
			auto it = array1.end();
			array1.insert(it, array2.at(i));
		}
	} else if ((matchpoint_right_timeline1 != (int)array1.size() - 1 && matchpoint_right_timeline2 == (int)array2.size() - 1)) {
		std::cerr << "[log] (Partial) Merging: The first timeline has more info at the end, so let's copy this" << std::endl;
		for (size_t i = matchpoint_right_timeline1 + 1; i < array1.size(); i++) {
			auto it = array2.end();
			array2.insert(it, array1.at(i));
		}
	}
}

void Detective::partialmerge_info_inbetween(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	assert(points.size() > 0);
	for (size_t i = 0; i < points.size() - 1; i++) {	// iterate over all couples of matches that are next each other on each timeline
		points = find_matchingpoints(array1, array2);
		int matchpoint_left_timeline1 = points.at(i).first;
		int matchpoint_left_timeline2 = points.at(i).second;
		int matchpoint_right_timeline1 = points.at(i + 1).first;
		int matchpoint_right_timeline2 = points.at(i + 1).second;
		if ((matchpoint_right_timeline1 - matchpoint_left_timeline1 == 1 && matchpoint_right_timeline2 - matchpoint_left_timeline2 != 1)) {
			std::cerr << "[log] (Partial) Merging: On the second timeline there is additional info, so let's copy this" << std::endl;
			auto it = array1.begin() + matchpoint_right_timeline1;
			for (int j = matchpoint_right_timeline2 - 1; j > matchpoint_left_timeline2; j--) {	// copy everything between thos points
				array1.insert(it, array2.at(j));
			}
		} else if ((matchpoint_right_timeline2 - matchpoint_left_timeline2 == 1 && matchpoint_right_timeline1 - matchpoint_left_timeline1 != 1)) {
			std::cerr << "[log] (Partial) Merging: On the first timeline there is additional info, so let's copy this" << std::endl;

			auto it = array2.begin() + matchpoint_right_timeline2;
			for (int j = matchpoint_right_timeline1 - 1; j > matchpoint_left_timeline1; j--) {
				it = array2.insert(it, array1.at(j));
			}
		}
	}
}

void Detective::write_timeline(const timeline & t, bool last)
{
	if (t.size() == 0) {
		std::cout << "[]";
	} else {
		std::cout << "[";
		for (size_t j = 0; j < t.size() - 1; j++) {
			std::cout << t.at(j) << ",";
		}
		std::cout << t.at(t.size() - 1) << "]";
	}
	if (!last) {
		std::cout << ",";
	}
	std::cout << std::endl;
}

void Detective::load_json(const char *filename)
{
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
	std::cerr << "[log] succesfully parsed " << filename << " as JSON format" << std::endl;
}

void Detective::write_json()
{
	// this function will just print all the timelines in JSON format
	std::sort(timelines.begin(), timelines.end(), smaller_timeline);
	std::cout << "[" << std::endl;
	for (size_t i = 0; i < timelines.size() - 1; i++) {
		timeline t = timelines.at(i);
		write_timeline(t, false);
	}
	write_timeline(timelines.at(timelines.size() - 1), true);
	std::cout << "]" << std::endl;
}

Detective::Detective(const char *filename)
{
	std::cerr << "[log] Created Detective object" << std::endl;
	load_json(filename);
}

bool
 Detective::can_timelines_merge(timeline & array1, timeline & array2)
{
	std::cerr << "[log] checking to see if timelines can be merged" << std::endl;
	matchpoints points = find_matchingpoints(array1, array2);
	if (points.size() == 0) {
		return false;
	}			// if there are no matching points then we can never merge
	if (points.size() == 1) {
		std::cerr << "[log] There is only one matching point" << std::endl;
		int matchpoint_timeline1 = points.at(0).first;
		int matchpoint_timeline2 = points.at(0).second;
		// false if they are both at the same edge or neither of them is at an edge => have to fulfull both requirements therefore &&
		bool both_at_same_edge = (matchpoint_timeline1 == 0 && matchpoint_timeline2 == 0)
		    || (matchpoint_timeline1 == (int)array1.size() - 1 && matchpoint_timeline2 == (int)array2.size() - 1);
		bool no_match_at_edge = (matchpoint_timeline1 != 0 && matchpoint_timeline1 != (int)array1.size() - 1 && matchpoint_timeline2 != 0 && matchpoint_timeline2 != (int)array2.size() - 1);
		if (both_at_same_edge && no_match_at_edge) {
			return false;
		}
	} else {
		for (size_t i = 0; i < points.size() - 1; i++) {
			if (unwanted_events_between_matchpoints_on_both_timelines(i, array1, array2)) {
				return false;
			}
		}
		if (conflicts_at_edge(array1, array2)) {
			return false;
		}
	}
	return true;
}

bool Detective::is_partial_merge_possible(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	// do checks here on if there is only 1 matchingpoint???

	if (points.size() == 0) {
		return false;
	}

	int matchpoint_left_timeline1 = points.at(0).first;
	int matchpoint_left_timeline2 = points.at(0).second;
	int matchpoint_right_timeline1 = points.at(points.size() - 1).first;
	int matchpoint_right_timeline2 = points.at(points.size() - 1).second;

	bool max_one_match_at_one_edge = (matchpoint_left_timeline1 == 0 && matchpoint_left_timeline2 != 0)
	    || (matchpoint_left_timeline1 != 0 && matchpoint_left_timeline2 == 0)
	    || (matchpoint_right_timeline1 != (int)array1.size() - 1 && matchpoint_right_timeline2 == (int)array2.size() - 1)
	    || (matchpoint_right_timeline1 == (int)array1.size() - 1 && matchpoint_right_timeline2 != (int)array2.size() - 1);
	if (max_one_match_at_one_edge) {
		// you don't need to check if the element left from it isn't a matching point since you only look a the extrema
		std::cout << "[log] a partial merge is possible since 1 of the timeline starts or ends with a matching point while the other one has non-matching events before that" << std::endl;
		return true;
	}
	// just check if the matching points on timeline 1 (or 2) are next to eachother while those on timeline 2 aren't e.g. [tan fight gunshot] and [sugar fight sweet gunshot]
	if ((matchpoint_right_timeline1 - matchpoint_left_timeline1 == 1 && matchpoint_right_timeline2 - matchpoint_left_timeline2 != 1)
	    || (matchpoint_right_timeline1 - matchpoint_left_timeline1 != 1 && matchpoint_right_timeline2 - matchpoint_left_timeline2 == 1)) {
		return true;
	}
	return false;
}

void Detective::merge_timelines(timeline & array1, timeline & array2)
{
	// try to make the first timeline as long as possible since the other one will be thrown away anyway
	std::cout << "[log] merging timelines : ";
	write_timeline(array1, true);
	std::cout << " and ";
	write_timeline(array2, true);
	std::cout << std::endl;

	matchpoints points = find_matchingpoints(array1, array2);
	if (points.size() == 0) {
		return;
	}

	partialmerge_longer_left(array1, array2);

	partialmerge_longer_right(array1, array2);

	partialmerge_info_inbetween(array1, array2);
}

void Detective::partially_merge_timelines(timeline & array1, timeline & array2)
{
	matchpoints points = find_matchingpoints(array1, array2);
	partialmerge_longer_left(array1, array2);
	partialmerge_longer_right(array1, array2);
	partialmerge_info_inbetween(array1, array2);
}

void Detective::detect()
{
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
			std::cout << "[log] We will try to merge ";
			write_timeline(timelines.at(i), false);
			std::cout << " with al the previous ones" << std::endl;

			size_t j = 0;
			bool merged;
			while (j < i) {
				if (can_timelines_merge(timelines.at(j), timelines.at(i))) {
					std::cout << "[log] They can do a full merge" << std::endl;

					std::cout << "BEFORE MERGE: ";
					write_timeline(timelines.at(j), true);
					std::cout << " and ";
					write_timeline(timelines.at(i), true);
					std::cout << std::endl;

					merge_timelines(timelines.at(j), timelines.at(i));
					changes = true;
					merged = true;

					std::cout << "AFTER MERGE: ";
					write_timeline(timelines.at(j), true);
					std::cout << " and ";
					write_timeline(timelines.at(i), true);
					std::cout << std::endl;

				} else if (is_partial_merge_possible(timelines.at(j), timelines.at(i))) {
					partially_merge_timelines(timelines.at(j), timelines.at(i));
					changes = true;
				}
				j++;
			}
			if (merged) {	// if merges have taken place the duplicate can be deleted
				auto it = timelines.begin();
				it += i;
				timelines.erase(it);
				i--;	// since you erased a timeline at that position is now another timeline
				std::cout << "[log] Since they could fully merge one of the timelines was deleted" << std::endl;
			}
			i++;
			more_then_one_timeline = timelines.size();
		}
	}
}

matchpoints Detective::find_matchingpoints(timeline array1, timeline array2)
{
	// function returns an array containing 2 arrays, the first array contains the indexes of all matching points in array1, the second array contains the indexes of all matching points in array2
	std::cout << "[log] looking for matching points in ";
	write_timeline(array1, false);
	std::cout << " and ";
	write_timeline(array2, false);
	std::cout << std::endl;
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
		std::cout << "[log] No matching points detected" << std::endl;
	} else {
		std::cout << "[log] The matching points are ";
		for (size_t i = 0; i < result.size(); i++) {
			std::cout << array1.at(result.at(i).first) << " ";
		}
		std::cout << std::endl;
	}
	return result;
}

static inline void ltrim(std::string & s)
{				// trim from start (in place)
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),[](unsigned char ch) {
					return !std::isspace(ch);}
		));
}

static inline void rtrim(std::string & s)
{				// trim from end (in place)
	s.erase(std::find_if(s.rbegin(), s.rend(),[](unsigned char ch) {
			     return !std::isspace(ch);}
		).base(), s.end());
}

static inline void trim(std::string & s)
{
	ltrim(s);
	rtrim(s);
}				// trim from both ends (in place)

bool smaller_timeline(const timeline & a, const timeline & b)
{
	return a.size() > b.size();
}

bool earlier_match(const std::pair < int, int >&a, const std::pair < int, int >&b)
{
	return a.first < b.first;
}
