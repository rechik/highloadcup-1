#ifndef HIGHLOAD_GET_USER_VISITS_H
#define HIGHLOAD_GET_USER_VISITS_H

#include <string>
#include <algorithm>

#include "../state.h"
#include "base.h"
#include "../utility.h"

using namespace std;

struct UserVisitItem
{
    int8_t mark;
    int visited_at;
    string place;

    string to_json() const
    {
        return (string) "{\"mark\":" + to_string(mark) + ",\"visited_at\":" + to_string(visited_at) + ",\"place\":\"" +
               place + "\"}";
    }
};

class GetUserVisitsRouteProcessor : public RouteProcessor
{
public:
    explicit GetUserVisitsRouteProcessor(int socket_fd) : RouteProcessor(socket_fd)
    {
    }

    void process(const char* str_user_id,
                 const char* str_from_date,
                 const char* str_to_date,
                 const char* country,
                 const char* str_to_distance)
    {
        if (!Utility::is_int(str_user_id))
        {
            handle_404();
            return;
        }
        int user_id = atoi(str_user_id);

        if (!state.users.contains(user_id))
        {
            handle_404();
            return;
        }

        if (str_from_date && !Utility::is_int(str_from_date)
            || str_to_date && !Utility::is_int(str_to_date)
            || str_to_distance && !Utility::is_int(str_to_distance))
        {
            handle_400();
            return;
        }

        int from_date = str_from_date == NULL ? INT_MIN : atoi(str_from_date) + 1;
        int to_date = str_to_date == NULL ? INT_MAX : atoi(str_to_date) - 1;
        int to_distance = str_to_distance == NULL ? INT_MAX : atoi(str_to_distance) - 1;

        auto &user = state.users[user_id];
        if (user == NULL)
            throw runtime_error("user is NULL");

        vector<UserVisitItem> visits;

        string s_country = Utility::urlDecode(country ? country : "");

        for (auto &visit : state.user_visits[user->id])
        {
            if (from_date <= visit->visited_at && visit->visited_at <= to_date)
            {
                auto &location = state.locations[visit->location];

                if (location->distance <= to_distance)
                {
                    if (country == NULL || location->country == s_country)
                    {
                        visits.push_back({
                                                 visit->mark,
                                                 visit->visited_at,
                                                 location->place
                                         });
                    }
                }
            }
        }
        sort(visits.begin(), visits.end(), [](UserVisitItem &a, UserVisitItem &b) {
            return a.visited_at < b.visited_at;
        });

        string body = "{\"visits\":[";
        bool first = true;
        for (auto &visit : visits)
        {
            if (!first)
                body += ",";
            first = false;
            body += visit.to_json();
        }
        body += "]}";
        handle_200(body.c_str(), body.size());
    }
};

#endif //HIGHLOAD_GET_USER_VISITS_H
