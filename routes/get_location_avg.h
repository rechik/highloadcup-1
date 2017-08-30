#ifndef HIGHLOAD_GET_LOCATION_AVG_H
#define HIGHLOAD_GET_LOCATION_AVG_H

#include <string>

#include "../state.h"
#include "base.h"
#include "../utility.h"

using namespace std;

class GetLocationAvgRouteProcessor : public RouteProcessor
{
public:
    explicit GetLocationAvgRouteProcessor(int socket_fd) : RouteProcessor(socket_fd) 
    {
    }

    void process(
            const char* str_id,
            const char* str_from_date,
            const char* str_to_date,
            const char* str_from_age,
            const char* str_to_age,
            const char* str_gender
    )
    {
        if (!Utility::is_int(str_id))
        {
            handle_404();
            return;
        }
        int location_id = atoi(str_id);

        if (location_id >= M_LOCATIONS_MAX_ID || state.locations[location_id] == NULL)
        {
            handle_404();
            return;
        }

        if (str_from_date && !Utility::is_int(str_from_date)
            || str_to_date && !Utility::is_int(str_to_date)
            || str_from_age && !Utility::is_int(str_from_age)
            || str_to_age && !Utility::is_int(str_to_age)
            || str_gender && (str_gender[0] != 'm' || str_gender[1] != 0) && (str_gender[0] != 'f' || str_gender[1] != 0)
                )
        {
            handle_400();
            return;
        }

        auto timestamp = time(NULL);
        int from_date = str_from_date == NULL ? INT_MIN : atoi(str_from_date) + 1;
        int to_date = str_to_date == NULL ? INT_MAX : atoi(str_to_date) - 1;

        tm* cur_time = localtime(&timestamp);
        long long from_birth = str_to_age == NULL ? LONG_LONG_MIN : _time_minus_years(*cur_time, atoi(str_to_age)) + 1;
        long long to_birth = str_from_age == NULL ? LONG_LONG_MAX : _time_minus_years(*cur_time, atoi(str_from_age)) - 1;

        int sum = 0;
        int count = 0;
        for(auto &visit : state.location_visits[location_id])
        {
            if (from_date <= visit->visited_at && visit->visited_at <= to_date)
            {
                auto &user = state.users[visit->user];
                if (from_birth <= user->birth_date && user->birth_date <= to_birth)
                {
                    if (str_gender == NULL || str_gender[0] == 'm' && user->gender == User::Male || str_gender[0] == 'f' && user->gender == User::Female)
                    {
                        sum += visit->mark;
                        count++;
                    }
                }
            }
        }

        if (count == 0)
        {
            sum = 0;
            count = 1;
        }

#define M_LOCATION_AVG_RESPONSE_PREFIX M_RESPONSE_200_PREFIX "15\n\n{\"avg\":"
        const int offset = M_STRLEN(M_LOCATION_AVG_RESPONSE_PREFIX);
        static thread_local char buf[offset + 20] = M_LOCATION_AVG_RESPONSE_PREFIX;

        sprintf(buf + offset, "%.5f}", 1.0 * sum / count + 1e-10);
        handle(buf, offset + 8);
    }

private:
    static time_t _time_minus_years(tm time, int years)
    {
        time.tm_year -= years;
        return mktime(&time);
    }
};

#endif //HIGHLOAD_GET_LOCATION_AVG_H
