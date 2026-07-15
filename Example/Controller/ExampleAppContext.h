#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7).
//
// ExampleAppContext bundles the in-memory storage the example menu tree
// (ExampleMenuTreeFactory.h) operates on: one InMemoryModel<T> per entity
// type plus the single global production FifoQueue, exactly as
// docs/feature/model.md section 6 and ../../CLAUDE.md ("영속성 없음") specify -
// no persistence adapter is supplied, so everything lives only in memory for
// the process lifetime.
//
// It also owns simple monotonically increasing id counters. IEntity<TId>
// requires ids to be assigned once at construction and never change
// afterwards (see ../../Model/IEntity.h) - since this skeleton's core does
// not provide an id generator (that is a domain concern), the example
// context supplies the simplest one that satisfies that contract.

#include "../Model/Order.h"
#include "../Model/ProductionQueueEntry.h"
#include "../Model/Sample.h"
#include "../../Model/FifoQueue.h"
#include "../../Model/InMemoryModel.h"

namespace ConsoleMVC::Example::Controller
{
    struct ExampleAppContext
    {
        ConsoleMVC::Model::InMemoryModel<Model::Sample> sampleModel;
        ConsoleMVC::Model::InMemoryModel<Model::Order> orderModel;
        ConsoleMVC::Model::FifoQueue<Model::ProductionQueueEntry> productionQueue;

        int nextSampleId = 1;
        int nextOrderId = 1;

        int TakeNextSampleId()
        {
            return nextSampleId++;
        }

        int TakeNextOrderId()
        {
            return nextOrderId++;
        }
    };
}
