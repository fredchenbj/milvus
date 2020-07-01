// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License.

#include "examples/utils/Utils.h"

#include <time.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "examples/utils/TimeRecorder.h"

namespace milvus_sdk {

constexpr int64_t SECONDS_EACH_HOUR = 3600;

#define BLOCK_SPLITER std::cout << "===========================================" << std::endl;

std::string
Utils::CurrentTime() {
    time_t tt;
    time(&tt);
    tt = tt + 8 * SECONDS_EACH_HOUR;
    tm t;
    gmtime_r(&tt, &t);

    std::string str = std::to_string(t.tm_year + 1900) + "_" + std::to_string(t.tm_mon + 1) + "_" +
                      std::to_string(t.tm_mday) + "_" + std::to_string(t.tm_hour) + "_" + std::to_string(t.tm_min) +
                      "_" + std::to_string(t.tm_sec);

    return str;
}

std::string
Utils::CurrentTmDate(int64_t offset_day) {
    time_t tt;
    time(&tt);
    tt = tt + 8 * SECONDS_EACH_HOUR;
    tt = tt + 24 * SECONDS_EACH_HOUR * offset_day;
    tm t;
    gmtime_r(&tt, &t);

    std::string str =
        std::to_string(t.tm_year + 1900) + "-" + std::to_string(t.tm_mon + 1) + "-" + std::to_string(t.tm_mday);

    return str;
}

void
Utils::Sleep(int seconds) {
    std::cout << "Waiting " << seconds << " seconds ..." << std::endl;
    sleep(seconds);
}

const std::string&
Utils::GenCollectionName() {
    static std::string s_id("C_" + CurrentTime());
    return s_id;
}

std::string
Utils::MetricTypeName(const milvus::MetricType& metric_type) {
    switch (metric_type) {
        case milvus::MetricType::L2:
            return "L2 distance";
        case milvus::MetricType::IP:
            return "Inner product";
        case milvus::MetricType::HAMMING:
            return "Hamming distance";
        case milvus::MetricType::JACCARD:
            return "Jaccard distance";
        case milvus::MetricType::TANIMOTO:
            return "Tanimoto distance";
        case milvus::MetricType::SUBSTRUCTURE:
            return "Substructure distance";
        case milvus::MetricType::SUPERSTRUCTURE:
            return "Superstructure distance";
        default:
            return "Unknown metric type";
    }
}

std::string
Utils::IndexTypeName(const milvus::IndexType& index_type) {
    switch (index_type) {
        case milvus::IndexType::FLAT:
            return "FLAT";
        case milvus::IndexType::IVFFLAT:
            return "IVFFLAT";
        case milvus::IndexType::IVFSQ8:
            return "IVFSQ8";
        case milvus::IndexType::RNSG:
            return "NSG";
        case milvus::IndexType::IVFSQ8H:
            return "IVFSQ8H";
        case milvus::IndexType::IVFPQ:
            return "IVFPQ";
        case milvus::IndexType::SPTAGKDT:
            return "SPTAGKDT";
        case milvus::IndexType::SPTAGBKT:
            return "SPTAGBKT";
        case milvus::IndexType::HNSW:
            return "HNSW";
        case milvus::IndexType::ANNOY:
            return "ANNOY";
        default:
            return "Unknown index type";
    }
}

void
Utils::PrintCollectionParam(const milvus::Mapping& mapping) {
    BLOCK_SPLITER
    std::cout << "Collection name: " << mapping.collection_name << std::endl;
    for (const auto& field : mapping.fields) {
        std::cout << "field_name: " << field->field_name;
        std::cout << "field_type: " << std::to_string((int)field->field_type);
        std::cout << "index_param: " << field->index_params;
        std::cout << "extra_param:" << field->extra_params;
    }
    BLOCK_SPLITER
}

void
Utils::PrintPartitionParam(const milvus::PartitionParam& partition_param) {
    BLOCK_SPLITER
    std::cout << "Collection name: " << partition_param.collection_name << std::endl;
    std::cout << "Partition tag: " << partition_param.partition_tag << std::endl;
    BLOCK_SPLITER
}

void
Utils::PrintIndexParam(const milvus::IndexParam& index_param) {
    BLOCK_SPLITER
    std::cout << "Index collection name: " << index_param.collection_name << std::endl;
    std::cout << "Index field name: " << index_param.field_name << std::endl;
    std::cout << "Index name: " << index_param.index_name << std::endl;
    std::cout << "Index extra_params: " << index_param.extra_params << std::endl;
    BLOCK_SPLITER
}

void
Utils::BuildEntities(int64_t from, int64_t to, std::vector<milvus::VectorData>& entity_array,
                     std::vector<int64_t>& entity_ids, int64_t dimension) {
    if (to <= from) {
        return;
    }

    entity_array.clear();
    entity_ids.clear();
    std::default_random_engine e;
    std::uniform_real_distribution<float> u(0, 1);
    for (int64_t k = from; k < to; k++) {
        milvus::VectorData vector_data;
        vector_data.float_data.resize(dimension);
        for (int64_t i = 0; i < dimension; i++) {
            vector_data.float_data[i] = (float)((k + 100) % (i + 1));
        }

        entity_array.emplace_back(vector_data);
        entity_ids.push_back(k);
    }
}

void
Utils::PrintSearchResult(const std::vector<std::pair<int64_t, milvus::VectorData>>& entity_array,
                         const milvus::TopKQueryResult& topk_query_result) {
    BLOCK_SPLITER
    std::cout << "Returned result count: " << topk_query_result.size() << std::endl;

    if (topk_query_result.size() != entity_array.size()) {
        std::cout << "ERROR: Returned result count not equal nq" << std::endl;
        return;
    }

    for (size_t i = 0; i < topk_query_result.size(); i++) {
        const milvus::QueryResult& one_result = topk_query_result[i];
        size_t topk = one_result.ids.size();
        auto search_id = entity_array[i].first;
        std::cout << "No." << i << " entity " << search_id << " top " << topk << " search result:" << std::endl;
        for (size_t j = 0; j < topk; j++) {
            std::cout << "\t" << one_result.ids[j] << "\t" << one_result.distances[j] << std::endl;
        }
    }
    BLOCK_SPLITER
}

void
Utils::CheckSearchResult(const std::vector<std::pair<int64_t, milvus::VectorData>>& entity_array,
                         const milvus::TopKQueryResult& topk_query_result) {
    BLOCK_SPLITER
    size_t nq = topk_query_result.size();
    for (size_t i = 0; i < nq; i++) {
        const milvus::QueryResult& one_result = topk_query_result[i];
        auto search_id = entity_array[i].first;

        uint64_t match_index = one_result.ids.size();
        for (uint64_t index = 0; index < one_result.ids.size(); index++) {
            if (search_id == one_result.ids[index]) {
                match_index = index;
                break;
            }
        }

        if (match_index >= one_result.ids.size()) {
            std::cout << "The topk result is wrong: not return search target in result set" << std::endl;
        } else {
            std::cout << "No." << i << " Check result successfully for target: " << search_id << " at top "
                      << match_index << std::endl;
        }
    }
    BLOCK_SPLITER
}

void
Utils::DoSearch(std::shared_ptr<milvus::Connection> conn, const std::string& collection_name,
                const std::vector<std::string>& partition_tags, int64_t top_k, int64_t nprobe,
                const std::vector<std::pair<int64_t, milvus::VectorData>>& entity_array,
                milvus::TopKQueryResult& topk_query_result) {
    topk_query_result.clear();

    std::vector<milvus::VectorData> temp_entity_array;
    for (auto& pair : entity_array) {
        temp_entity_array.push_back(pair.second);
    }

    {
        BLOCK_SPLITER
        JSON json_params = {{"nprobe", nprobe}};
        milvus_sdk::TimeRecorder rc("Search");
        BLOCK_SPLITER
    }

    PrintSearchResult(entity_array, topk_query_result);
    CheckSearchResult(entity_array, topk_query_result);
}

void
Utils::ConstructVector(uint64_t nq, uint64_t dimension, std::vector<milvus::VectorData>& query_vector) {
    query_vector.resize(nq);
    std::default_random_engine e;
    std::uniform_real_distribution<float> u(0, 1);
    for (uint64_t i = 0; i < nq; ++i) {
        query_vector[i].float_data.resize(dimension);
        for (uint64_t j = 0; j < dimension; ++j) {
            query_vector[i].float_data[j] = (float)((j + 100) % (i + 1));
        }
    }
}

std::vector<milvus::LeafQueryPtr>
Utils::GenLeafQuery() {
    // Construct TermQuery
    uint64_t row_num = 10000;
    std::vector<int64_t> field_value;
    field_value.resize(row_num);
    for (uint64_t i = 0; i < row_num; ++i) {
        field_value[i] = i;
    }
    milvus::TermQueryPtr tq = std::make_shared<milvus::TermQuery>();
    tq->field_name = "field_1";
    tq->int_value = field_value;

    // Construct RangeQuery
    milvus::CompareExpr ce1 = {milvus::CompareOperator::LTE, "100000"}, ce2 = {milvus::CompareOperator::GTE, "1"};
    std::vector<milvus::CompareExpr> ces{ce1, ce2};
    milvus::RangeQueryPtr rq = std::make_shared<milvus::RangeQuery>();
    rq->field_name = "field_2";
    rq->compare_expr = ces;

    // Construct VectorQuery
    uint64_t NQ = 10;
    uint64_t DIMENSION = 128;
    uint64_t NPROBE = 32;
    milvus::VectorQueryPtr vq = std::make_shared<milvus::VectorQuery>();
    ConstructVector(NQ, DIMENSION, vq->query_vector);
    vq->field_name = "field_3";
    vq->topk = 10;
    JSON json_params = {{"nprobe", NPROBE}};
    vq->extra_params = json_params.dump();

    std::vector<milvus::LeafQueryPtr> lq;
    milvus::LeafQueryPtr lq1 = std::make_shared<milvus::LeafQuery>();
    milvus::LeafQueryPtr lq2 = std::make_shared<milvus::LeafQuery>();
    milvus::LeafQueryPtr lq3 = std::make_shared<milvus::LeafQuery>();
    lq.emplace_back(lq1);
    lq.emplace_back(lq2);
    lq.emplace_back(lq3);
    lq1->term_query_ptr = tq;
    lq2->range_query_ptr = rq;
    lq3->vector_query_ptr = vq;

    lq1->query_boost = 1.0;
    lq2->query_boost = 2.0;
    lq3->query_boost = 3.0;
    return lq;
}

void
Utils::GenDSLJson(nlohmann::json& dsl_json, nlohmann::json& vector_param_json) {
    uint64_t row_num = 10000;
    std::vector<int64_t> term_value;
    term_value.resize(row_num);
    for (uint64_t i = 0; i < row_num; ++i) {
        term_value[i] = i;
    }

    nlohmann::json bool_json, term_json, range_json, vector_json;
    term_json["term"]["field_name"] = "field_1";
    term_json["term"]["values"] = term_value;
    bool_json["must"].push_back(term_json);

    range_json["range"]["field_name"] = "field_1";
    nlohmann::json comp_json;
    comp_json["gte"] = "0";
    comp_json["lte"] = "100000";
    range_json["range"]["values"] = comp_json;
    bool_json["must"].push_back(range_json);

    std::string placeholder = "placeholder_1";
    vector_json["vector"] = placeholder;
    bool_json["must"].push_back(vector_json);

    dsl_json["bool"] = bool_json;

    nlohmann::json vector_extra_params;
    int64_t topk = 10;
    vector_param_json[placeholder]["field_name"] = "field_3";
    vector_param_json[placeholder]["topk"] = topk;
    vector_extra_params["nprobe"] = 64;
    vector_param_json[placeholder]["params"] = vector_extra_params;
}

void
Utils::PrintTopKQueryResult(milvus::TopKQueryResult& topk_query_result) {
    for (size_t i = 0; i < topk_query_result.size(); i++) {
        auto field_value = topk_query_result[i].field_value;
        for (auto& int32_it : field_value.int32_value) {
            std::cout << int32_it.first << ":";
            for (auto& data : int32_it.second) {
                std::cout << " " << data;
            }
            std::cout << std::endl;
        }
        for (auto& int64_it : field_value.int64_value) {
            std::cout << int64_it.first << ":";
            for (auto& data : int64_it.second) {
                std::cout << " " << data;
            }
            std::cout << std::endl;
        }
        for (auto& float_it : field_value.float_value) {
            std::cout << float_it.first << ":";
            for (auto& data : float_it.second) {
                std::cout << " " << data;
            }
            std::cout << std::endl;
        }
        for (auto& double_it : field_value.double_value) {
            std::cout << double_it.first << ":";
            for (auto& data : double_it.second) {
                std::cout << " " << data;
            }
            std::cout << std::endl;
        }
        for (size_t j = 0; j < topk_query_result[i].ids.size(); j++) {
            std::cout << topk_query_result[i].ids[j] << "  ---------  " << topk_query_result[i].distances[j]
                      << std::endl;
        }
    }
}

}  // namespace milvus_sdk
