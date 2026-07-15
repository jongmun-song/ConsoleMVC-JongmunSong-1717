#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)").
//
// ServiceOutcome is the result type shared by the Example-domain combination
// services (OrderApprovalService.h, ProductionOrchestrator.h): a boolean
// success flag plus a human-readable reason, mirroring the "expected failure
// is an ordinary result, not an exception" policy already used by
// StatefulModel::TryTransition and QuantityGuard::TryIncrease/TryDecrease
// (see docs/feature/model.md section 3.4 and docs/feature/controller.md
// section 4).
//
// This is intentionally a separate, Controller-agnostic type rather than a
// reuse of Controller::ActionOutcome: docs/feature/model.md section 4 states
// that the Model layer does not perform console I/O or otherwise depend on
// Controller, so these Model-level services must not include
// Controller/IController.h. Example/Controller's action handlers translate a
// ServiceOutcome into a Controller::ActionOutcome at the call site instead.

#include <string>

namespace ConsoleMVC::Example::Model
{
    struct ServiceOutcome
    {
        bool succeeded = false;
        std::string message;
    };
}
