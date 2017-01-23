//
// Created by liaosiwei on 17/1/14.
//

#ifndef OKCOIN_BOT_PIPELINE_H
#define OKCOIN_BOT_PIPELINE_H

#include <unordered_map>
#include <util/queue.h>
#include <rapidjson/document.h>

class Pipeline {
public:
    using pipe = Queue<std::shared_ptr<rapidjson::Document>>;
    using pipe_ptr = Queue<std::shared_ptr<rapidjson::Document>>*;
    pipe& get_queue(const std::string& name) {
        {
            std::lock_guard<std::mutex> locker(lock);
            if (queue_map.count(name) == 0) {
                queue_map[name] = new Queue<std::shared_ptr<rapidjson::Document>>();
            }
        }
        return *(queue_map[name]);
    }
private:
    std::mutex lock;
    std::unordered_map<std::string, pipe_ptr> queue_map;
};

#endif //OKCOIN_BOT_PIPELINE_H
