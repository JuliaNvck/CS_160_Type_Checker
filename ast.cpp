#include "ast.hpp"
#include <sstream>
#include <algorithm>


// --- Forward Declarations ---
std::shared_ptr<Type> buildType(const nlohmann::json& j);
std::unique_ptr<Exp> buildExp(const nlohmann::json& j);
std::unique_ptr<Place> buildPlace(const nlohmann::json& j);
std::unique_ptr<Stmt> buildStmt(const nlohmann::json& j);
Decl buildDecl(const nlohmann::json& j);
std::unique_ptr<FunctionDef> buildFunctionDef(const nlohmann::json& j);
std::unique_ptr<StructDef> buildStructDef(const nlohmann::json& j);
Extern buildExtern(const nlohmann::json& j);
std::unique_ptr<Program> buildProgram(const nlohmann::json& j);
std::unique_ptr<FunCall> buildFunCall(const nlohmann::json& j);

// Type Implementations

std::string FnType::toString() const {
    std::stringstream ss;
    ss << "fn(<";
    for (size_t i = 0; i < paramTypes.size(); ++i) {
        ss << paramTypes[i]->toString() << (i == paramTypes.size() - 1 ? "" : ",");
    }
    ss << ">, " << returnType->toString() << ")";
    return ss.str();
}

// Define equals methods moved from header because of circular dependency
bool NilType::equals(const Type& other) const {
    // nil is eq to nil, ptr(_), or array(_)
    // Now PtrType and ArrayType are fully defined
    return dynamic_cast<const NilType*>(&other) != nullptr ||
           dynamic_cast<const PtrType*>(&other) != nullptr ||
           dynamic_cast<const ArrayType*>(&other) != nullptr;
}

bool StructType::equals(const Type& other) const {
    // nil is not eq to struct types
    if (dynamic_cast<const NilType*>(&other)) {
        return false;
    }
    if (const auto* other_struct = dynamic_cast<const StructType*>(&other)) {
        return name == other_struct->name;
    }
    return false;
}

bool ArrayType::equals(const Type& other) const {
    if (dynamic_cast<const NilType*>(&other)) {
        return true; // array(_) eq nil
    }
    if (const auto* other_array = dynamic_cast<const ArrayType*>(&other)) {
        // Now TypePtrEqual can be used as all types are defined
        return TypePtrEqual()(elementType, other_array->elementType);
    }
    return false;
}

bool PtrType::equals(const Type& other) const {
    if (dynamic_cast<const NilType*>(&other)) {
        return true; // ptr(_) eq nil
    }
    if (const auto* other_ptr = dynamic_cast<const PtrType*>(&other)) {
        // Now TypePtrEqual can be used as all types are defined
         return TypePtrEqual()(pointeeType, other_ptr->pointeeType);
    }
    return false;
}

bool FnType::equals(const Type& other) const {
    // nil is not eq to function types
    if (dynamic_cast<const NilType*>(&other)) {
        return false;
    }
    if (const auto* other_fn = dynamic_cast<const FnType*>(&other)) {
        if (paramTypes.size() != other_fn->paramTypes.size()) {
            return false;
        }
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            // Use custom comparison
            if (!TypePtrEqual()(paramTypes[i], other_fn->paramTypes[i])) {
                return false;
            }
        }
        // Use custom comparison
        return TypePtrEqual()(returnType, other_fn->returnType);
    }
    return false;
}

// Type Helper Implementations

// Type equality function eq(τ₁, τ₂)
// compares types s.t. two types are eq iff they are the same
// type or if one is a pointer or array type and the other is nil.
bool typeEq(const std::shared_ptr<Type>& t1, const std::shared_ptr<Type>& t2) {
    if (!t1 || !t2) return false;
    // Handle nil comparison
    if (dynamic_cast<NilType*>(t1.get())) {
        return dynamic_cast<NilType*>(t2.get()) != nullptr ||
               dynamic_cast<PtrType*>(t2.get()) != nullptr ||
               dynamic_cast<ArrayType*>(t2.get()) != nullptr;
    }
     if (dynamic_cast<NilType*>(t2.get())) {
        // t1 is not Nil, check if it's Ptr or Array
        return dynamic_cast<PtrType*>(t1.get()) != nullptr ||
               dynamic_cast<ArrayType*>(t1.get()) != nullptr;
    }
    // use virtual equals for deep comparison
    return t1->equals(*t2);
}

// pick_nonnil helper function
// Type × Type → Type that takes two types and returns one of the
// arguments that isn’t nil if possible; if both arguments are nil then it returns nil.
std::shared_ptr<Type> pickNonNil(const std::shared_ptr<Type>& t1, const std::shared_ptr<Type>& t2) {
    if (!dynamic_cast<NilType*>(t1.get())) {
        return t1;
    }
     // If t1 is Nil, return t2 (which might also be Nil)
    return t2;
}

// Print Implementations

void UnOp::print(std::ostream& os) const {
    os << "UnOp(";
    switch (op) {
        case UnaryOp::Neg: os << "Neg"; break;
        case UnaryOp::Not: os << "Not"; break;
    }
    os << ", " << exp << ")";
}
std::string UnOp::toString() const {
     std::string opStr;
     switch(op) {
         case UnaryOp::Neg: opStr = "-"; break;
         case UnaryOp::Not: opStr = "!"; break;
     }
     return opStr + exp->toString();
}


void BinOp::print(std::ostream& os) const {
    os << "BinOp { op: ";
    switch (op) {
        case BinaryOp::Add: os << "Add"; break;
        case BinaryOp::Sub: os << "Sub"; break;
        case BinaryOp::Mul: os << "Mul"; break;
        case BinaryOp::Div: os << "Div"; break;
        case BinaryOp::And: os << "And"; break;
        case BinaryOp::Or: os << "Or"; break;
        case BinaryOp::Eq: os << "Eq"; break;
        case BinaryOp::NotEq: os << "NotEq"; break;
        case BinaryOp::Lt: os << "Lt"; break;
        case BinaryOp::Lte: os << "Lte"; break;
        case BinaryOp::Gt: os << "Gt"; break;
        case BinaryOp::Gte: os << "Gte"; break;
    }
    os << ", left: " << left << ", right: " << right << " }";
}
std::string BinOp::toString() const {
     std::string opStr;
     switch(op) {
        case BinaryOp::Add: opStr = "+"; break;
        case BinaryOp::Sub: opStr = "-"; break;
        case BinaryOp::Mul: opStr = "*"; break;
        case BinaryOp::Div: opStr = "/"; break;
        case BinaryOp::Eq: opStr = "=="; break;
        case BinaryOp::NotEq: opStr = "!="; break;
        case BinaryOp::Lt: opStr = "<"; break;
        case BinaryOp::Lte: opStr = "<="; break;
        case BinaryOp::Gt: opStr = ">"; break;
        case BinaryOp::Gte: opStr = ">="; break;
        case BinaryOp::And: opStr = "&&"; break;
        case BinaryOp::Or: opStr = "||"; break;
     }
     return "(" + left->toString() + " " + opStr + " " + right->toString() + ")"; // Added parens for clarity
}


void FunCall::print(std::ostream& os) const {
    os << "FunCall { callee: " << callee << ", args: [";
    for (size_t i = 0; i < args.size(); ++i) {
        os << args[i];
        if (i < args.size() - 1) os << ", ";
    }
    os << "] }";
}
std::string FunCall::toString() const {
    std::string s = callee->toString() + "(";
    for(size_t i = 0; i < args.size(); ++i) {
        s += args[i]->toString();
        if (i < args.size() - 1) s += ", ";
    }
    s += ")";
    return s;
}

void If::print(std::ostream& os) const {
    os << "If { guard: " << guard << ", tt: " << tt;
    os << ", ff: ";
    if (ff) {
        os << (*ff);
    } else {
        os << "<None>";
    }
    os << " }";
}

void StructDef::print(std::ostream& os) const {
    os << "Struct { name: \"" << name << "\", fields: {";
    for (size_t i = 0; i < fields.size(); ++i) {
        fields[i].print(os); // Decl has print
        if (i < fields.size() - 1) os << ", ";
    }
    os << "} }";
}

void Extern::print(std::ostream& os) const {
     os << "Extern { name: \"" << name << "\", prms: [";
        for (size_t i = 0; i < param_types.size(); ++i) {
            os << param_types[i];
            if (i < param_types.size() - 1) os << ", ";
        }
        os << "], rettyp: " << rettype << " }";
}

void FunctionDef::print(std::ostream& os) const {
    os << "Function { name: \"" << name << "\", ";
    os << "prms: [";
    for (size_t i = 0; i < params.size(); ++i) {
        params[i].print(os); // Decl has print
        if (i < params.size() - 1) os << ", ";
    }
    os << "], rettyp: " << rettype;
    os << ", locals: {";
    for (size_t i = 0; i < locals.size(); ++i) {
        locals[i].print(os);
        if (i < locals.size() - 1) os << ", ";
    }
    os << "}, ";
    os << "body: " << body << " }"; // Use operator<< for unique_ptr<Stmt>
}

void Program::print(std::ostream& os) const {
    os << "Program { structs: {";
    for (size_t i = 0; i < structs.size(); ++i) {
        os << structs[i];
        if (i < structs.size() - 1) os << ", ";
    }
    os << "}, externs: {";
     for (size_t i = 0; i < externs.size(); ++i) {
        externs[i].print(os);
        if (i < externs.size() - 1) os << ", ";
    }
    os << "}, functions: {";
    for (size_t i = 0; i < functions.size(); ++i) {
        os << functions[i];
        if (i < functions.size() - 1) os << ", ";
    }
    os << "}}";
}


// AST Node Check Implementations

// Γ(name) = τ
// Γ,∆ ⊢Id(name) : τ
std::shared_ptr<Type> Id::check(const Gamma& gamma, const Delta& delta) const {
    // identifier name is mapped to the type τ in current scope gamma
    auto it = gamma.find(name);
    if (it != gamma.end()) {
        // found in gamma
        return it->second;
    } else {
        // not found in gamma
        throw TypeError("id " + name + " does not exist in this scope");
    }
}

// n ≥0
// Γ,∆ ⊢Num(n) : int
std::shared_ptr<Type> Num::check(const Gamma& gamma, const Delta& delta) const {
    // Rule NUM
    if (value >= 0) {
        // valid number
        return std::make_shared<IntType>();
    } else {
        throw TypeError("negative number " + std::to_string(value) + " is not allowed");
    }
}

// 
// Γ,∆ ⊢Nil : nil
std::shared_ptr<Type> NilExp::check(const Gamma& gamma, const Delta& delta) const {
    return std::make_shared<NilType>();
}

// Γ,∆ ⊢e : ptr(τ)
// Γ,∆ ⊢Deref(e) : τ
std::shared_ptr<Type> Deref::check(const Gamma& gamma, const Delta& delta) const {
    // Check the type of the inner expression 'e'
    auto pointee = exp->check(gamma, delta);
    // Check if the resulting type is actually a pointer type
    if (auto ptrType = std::dynamic_pointer_cast<PtrType>(pointee)) {
        return ptrType->pointeeType;
    }
    // Premise failed: The type was not a PtrType
    throw TypeError("non-pointer type " + pointee->toString() + " for dereference '" + toString() + "'");
}

// Γ,∆ ⊢arr : array(τ) Γ,∆ ⊢idx : int
// Γ,∆ ⊢ArrayAccess(arr,idx) : τ
std::shared_ptr<Type> ArrayAccess::check(const Gamma& gamma, const Delta& delta) const {
    auto arrType = array->check(gamma, delta);
    auto idxType = index->check(gamma, delta);

    if (!typeEq(idxType, std::make_shared<IntType>())) {
         throw TypeError("non-int index type " + idxType->toString() + " for array access '" + toString() + "'");
    }

    if (auto actualArrayType = std::dynamic_pointer_cast<ArrayType>(arrType)) {
        return actualArrayType->elementType;
    }
    if (typeEq(arrType, std::make_shared<NilType>())) {
         throw TypeError("cannot access element of 'nil' array in '" + toString() + "'");
    }

    throw TypeError("non-array type " + arrType->toString() + " for array access '" + toString() + "'");
}

// Γ,∆ ⊢ptr : ptr(struct(id)) ∆(id)(fld) = τ
// Γ,∆ ⊢FieldAccess(ptr,fld) : τ
std::shared_ptr<Type> FieldAccess::check(const Gamma& gamma, const Delta& delta) const {
    // Check the type of the expression 'ptr' (the expression before the '.')
    auto baseType = ptr->check(gamma, delta); // The expression giving the pointer
    // Verify that baseType is a pointer type
    auto ptrType = std::dynamic_pointer_cast<PtrType>(baseType);

    if (!ptrType) {
        // Premise 1 failed: The type is not a pointer.
        throw TypeError("<" + baseType->toString() + "> is not a struct pointer type in field access '" + toString() + "'");
    }
    // Verify that the type pointed to is specifically a struct type, struct(id)
    auto structPtrType = std::dynamic_pointer_cast<StructType>(ptrType->pointeeType);
    if (!structPtrType) {
        // Premise 1 failed: The pointer does not point to a struct.
         throw TypeError("pointer type <" + baseType->toString() + "> does not point to a struct in field access '" + toString() + "'");
    }
    // Look up the struct name 'id' in the Delta environment
    if (!delta.count(structPtrType->name)) {
        // Premise 2 failed: Struct definition not found in Delta.
         throw TypeError("non-existent struct type " + structPtrType->name + " in field access '" + toString() + "'");
    }

    // Look up the field name 'fld' within the found struct's field map
    const auto& fields = delta.at(structPtrType->name);
    if (!fields.count(field)) {
         throw TypeError("non-existent field " + structPtrType->name + "::" + field + " in field access '" + toString() + "'");
    }
    // 'field' is the member variable holding the field name string
    return fields.at(field);
}

// Γ,∆ ⊢g : int Γ,∆ ⊢tt : τ1 Γ,∆ ⊢ff : τ2 eq(τ1,τ2) τ = pick-nonnil(τ1,τ2)
// Γ,∆ ⊢Select(g,tt,ff) : τ
std::shared_ptr<Type> Select::check(const Gamma& gamma, const Delta& delta) const {
    auto guardType = guard->check(gamma, delta);
    if (!typeEq(guardType, std::make_shared<IntType>())) {
         throw TypeError("non-int type " + guardType->toString() + " for select guard '" + guard->toString() + "'");
    }

    auto ttType = tt->check(gamma, delta);
    auto ffType = ff->check(gamma, delta);

    if (!typeEq(ttType, ffType)) {
         throw TypeError("incompatible types " + ttType->toString() + " vs " + ffType->toString() + " in select branches '" + tt->toString() + "' vs '" + ff->toString() + "'");
    }

    return pickNonNil(ttType, ffType);
}

// Γ,∆ ⊢e : int
// Γ,∆ ⊢Unop(op,e) : int
std::shared_ptr<Type> UnOp::check(const Gamma& gamma, const Delta& delta) const {
    // Rule UNOP
    auto operandType = exp->check(gamma, delta);
    if (!typeEq(operandType, std::make_shared<IntType>())) {
         throw TypeError("non-int operand type " + operandType->toString() + " in unary op '" + toString() + "'");
    }
    return std::make_shared<IntType>();
}

// Rules EQ/NEQ and BINOP-REST
std::shared_ptr<Type> BinOp::check(const Gamma& gamma, const Delta& delta) const {
    auto leftType = left->check(gamma, delta);
    auto rightType = right->check(gamma, delta);

    // EQ/NEQ
    // op ∈{Equal,NotEq} Γ,∆ ⊢left : τ1 Γ,∆ ⊢right : τ2 eq(τ1,τ2) τ1,τ2 ̸∈{struct( ),fn(, )}
    // Γ,∆ ⊢Binop(op,left,right) : int
    if (op == BinaryOp::Eq || op == BinaryOp::NotEq) {
        if (!typeEq(leftType, rightType)) {
            throw TypeError("incompatible types " + leftType->toString() + " vs " + rightType->toString() + " in binary op '" + toString() + "'");
        }
        if (dynamic_cast<StructType*>(leftType.get()) || dynamic_cast<FnType*>(leftType.get())) {
             throw TypeError("invalid type " + leftType->toString() + " used in binary op '" + toString() + "'");
        }
        if (dynamic_cast<StructType*>(rightType.get()) || dynamic_cast<FnType*>(rightType.get())) {
             throw TypeError("invalid type " + rightType->toString() + " used in binary op '" + toString() + "'");
        }
        return std::make_shared<IntType>();
    } else {
        // BINOP-REST applies
        // op ̸∈{Equal,NotEq} Γ,∆ ⊢left : int Γ,∆ ⊢right : int
        // Γ,∆ ⊢Binop(op,left,right) : int
        if (!typeEq(leftType, std::make_shared<IntType>())) {
            throw TypeError("non-int type " + leftType->toString() + " for left operand of binary op '" + toString() + "'");
        }
         if (!typeEq(rightType, std::make_shared<IntType>())) {
            throw TypeError("non-int type " + rightType->toString() + " for right operand of binary op '" + toString() + "'");
        }
        return std::make_shared<IntType>();
    }
} // add error for different op?

// typ ̸∈{nil,fn(, )}
// Γ,∆ ⊢NewSingle(typ) : ptr(typ) 
std::shared_ptr<Type> NewSingle::check(const Gamma& gamma, const Delta& delta) const {
    if (dynamic_cast<NilType*>(type.get()) || dynamic_cast<FnType*>(type.get())) {
        throw TypeError("invalid type used for allocation '" + toString() + "'");
    }
    // if struct type, does it exist in Delta? check it is defined
     if (auto st = std::dynamic_pointer_cast<StructType>(type)) {
         if (delta.find(st->name) == delta.end()) {
             throw TypeError("allocating non-existent struct type '" + toString() + "'");
         }
     }

    return std::make_shared<PtrType>(type);
}

// Γ,∆ ⊢amt : int typ ̸∈{nil,fn(, ),struct( )}
// Γ,∆ ⊢NewArray(typ,amt) : array(typ)
std::shared_ptr<Type> NewArray::check(const Gamma& gamma, const Delta& delta) const {
    auto amtType = size->check(gamma, delta);
    if (!typeEq(amtType, std::make_shared<IntType>())) {
        throw TypeError("non-int type " + amtType->toString() + " used for second argument of allocation '" + toString() + "'");
    }
    // Check if type is nil, fn, or struct
     if (dynamic_cast<NilType*>(type.get()) || dynamic_cast<FnType*>(type.get()) || dynamic_cast<StructType*>(type.get())) {
        throw TypeError("invalid type used for first argument of allocation '" + toString() + "'");
    }

    return std::make_shared<ArrayType>(type);
}

// Check for FunCall (used by CallExp and CallStmt)
// callee ̸= main ∀(e,τ1) ∈zip(args,⃗τ).[Γ,∆ ⊢e : τ2 ∧eq(τ1,τ2)]
// Γ,∆ ⊢callee : fn(⃗τ,τ′) ∨Γ,∆ ⊢callee : ptr(fn(⃗τ,τ′))
// Γ,∆ ⊢FunCall(callee,args) : τ′
std::shared_ptr<Type> FunCall::check(const Gamma& gamma, const Delta& delta) const {
    // Get the type of the expression being called
    auto calleeType = callee->check(gamma, delta);
    std::shared_ptr<FnType> funcType = nullptr;

    // Direct call (identifier)? Need special handling for 'main'
    if (auto idExp = dynamic_cast<Id*>(callee.get())) { // Check direct Id, not Val(Id)
        if (idExp->name == "main") {
            throw TypeError("trying to call 'main'");
        }
        // Look up the Id's name in Gamma
        if (gamma.count(idExp->name)) {
            auto potentialFnType = gamma.at(idExp->name);
             // Externs have type fn(...), internal functions have type ptr(fn(...))
            if (auto directFn = std::dynamic_pointer_cast<FnType>(potentialFnType)) {
                funcType = directFn; // Extern call
            } else if (auto ptrFn = std::dynamic_pointer_cast<PtrType>(potentialFnType)) {
                 funcType = std::dynamic_pointer_cast<FnType>(ptrFn->pointeeType); // Internal call
            }
        }
     } else if (auto valExp = dynamic_cast<Val*>(callee.get())) { // Check for Val(Id) too. Val(Id) when an Id is used as an expression
         if (auto idPlace = dynamic_cast<Id*>(valExp->place.get())) {
             if (idPlace->name == "main") {
                 throw TypeError("trying to call 'main'");
             }
             if (gamma.count(idPlace->name)) {
                 auto potentialFnType = gamma.at(idPlace->name);
                 if (auto directFn = std::dynamic_pointer_cast<FnType>(potentialFnType)) {
                     funcType = directFn;
                 } else if (auto ptrFn = std::dynamic_pointer_cast<PtrType>(potentialFnType)) {
                     funcType = std::dynamic_pointer_cast<FnType>(ptrFn->pointeeType);
                 }
             }
         }
     }


    // Indirect call (pointer)?
    // covers cases like (*ptr_to_func)(arg) or obj.func_ptr_field(arg)
    if (!funcType) {
        // Check if 'calleeType' is PtrType(...)
        if (auto ptrFn = std::dynamic_pointer_cast<PtrType>(calleeType)) {
            // Check if it points to FnType(...)
            funcType = std::dynamic_pointer_cast<FnType>(ptrFn->pointeeType);
        }
    }

    if (!funcType) { // Check if a valid function type (FnType) was found.
         throw TypeError("trying to call type " + calleeType->toString() + " as function pointer in call '" + toString() + "'");
    }

    // Check number of arguments: Compare number of args provided vs. expected parameters
    if (args.size() != funcType->paramTypes.size()) {
         throw TypeError("incorrect number of arguments (" + std::to_string(args.size()) + " vs " + std::to_string(funcType->paramTypes.size()) + ") in call '" + toString() + "'");
    }

    // Check argument types: Loop through provided args and expected param types.
    for (size_t i = 0; i < args.size(); ++i) {
        auto argType = args[i]->check(gamma, delta);
        const auto& paramType = funcType->paramTypes[i];
        if (!typeEq(argType, paramType)) {
             throw TypeError("incompatible argument type " + argType->toString() + " vs parameter type " + paramType->toString() + " for argument '" + args[i]->toString() + "' in call '" + toString() + "'");
        }
    }
    // All Premises Passed: Return the function's return type (τ').
    return funcType->returnType;
}


// Statement Check Implementations

// Γ,Δ,τr,loop ⊢ stmt : ok(ret1)   Γ,Δ,τr,loop ⊢ stmts : ok(ret2)   ret = ret1 ∨ ret2
//                      Γ,Δ,τr,loop ⊢ stmt; stmts : ok(ret)
bool Stmts::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    // Tracks if any statement encountered so far in this sequence *definitely* returns.
    bool definitelyReturns = false;

    // Iterate through each statement in the sequence.
    for (const auto& stmt : statements) {
        // If a previous statement in this block already guaranteed a return,
        // subsequent statements are unreachable for the purpose of satisfying the function's "must return" requirement
        if (definitelyReturns) {
            // Check the statement but ignore its return status - already know this path guarantees a return earlier.
            stmt->check(gamma, delta, returnType, inLoop);
        } else {
            // If this stmt->check returns true, then 'definitelyReturns' becomes true
            definitelyReturns = stmt->check(gamma, delta, returnType, inLoop);
        }
    }

    // The sequence as a whole definitely returns if any statement within it returns along the current execution path.
    return definitelyReturns;
}

// Γ,∆ ⊢lhs : τ1 Γ,∆ ⊢rhs : τ2 eq(τ1,τ2) τ1 ̸∈{nil,struct( ),fn(, )}
// Γ,∆,τr ,loop ⊢Assign(lhs,rhs) : ok(false) 
bool Assign::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    // Rule ASSIGN
    auto lhsType = place->check(gamma, delta);
    auto rhsType = exp->check(gamma, delta);

    // Check for invalid types on LHS (struct/fn/nil)
    if (dynamic_cast<StructType*>(lhsType.get()) || dynamic_cast<FnType*>(lhsType.get()) || dynamic_cast<NilType*>(lhsType.get())) {
        throw TypeError("invalid type " + lhsType->toString() + " for left-hand side of assignment '" + place->toString() + " = " + exp->toString() + "'");
    }
    // Check for invalid types on RHS (struct/fn/nil) according to rule image
    if (dynamic_cast<StructType*>(rhsType.get()) || dynamic_cast<FnType*>(rhsType.get()) || dynamic_cast<NilType*>(rhsType.get())) {
        throw TypeError("invalid type " + rhsType->toString() + " for right-hand side of assignment '" + place->toString() + " = " + exp->toString() + "'");
    }

    if (!typeEq(lhsType, rhsType)) {
         throw TypeError("incompatible types " + lhsType->toString() + " vs " + rhsType->toString() + " for assignment '" + place->toString() + " = " + exp->toString() + "'");
    }
    return false; // Assignment never definitely returns
}

// Γ,∆ ⊢funcall : τ
// Γ,∆,τr ,loop ⊢Call(funcall) : ok(false) 
bool CallStmt::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    fun_call->check(gamma, delta); // Check the call expression (throws on error)
    return false; // Call statement never definitely returns
}

// Γ,∆ ⊢g : int Γ,∆,τr ,loop ⊢tt : ok(ret1) Γ,∆,τr ,loop ⊢ff : ok(ret1)  ret= ret1 ⊗ret2
// Γ,∆,τr ,loop ⊢If(g,tt,ff) : ok(ret)
bool If::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    auto guardType = guard->check(gamma, delta);
    if (!typeEq(guardType, std::make_shared<IntType>())) {
         throw TypeError("non-int type " + guardType->toString() + " for if guard '" + guard->toString() + "'");
    }

    // Store whether this branch definitely returns.
    bool ttReturns = tt->check(gamma, delta, returnType, inLoop); 
    bool ffReturns = false;
    if (ff.has_value()) {
        ffReturns = (*ff)->check(gamma, delta, returnType, inLoop);
    } else {
        ffReturns = false; // No else branch means it doesn't definitely return
    }

    return ttReturns && ffReturns; // Definitely returns only if *both* branches definitely return
}

// Γ,∆ ⊢g : int Γ,∆,τr ,true ⊢body : ok(ret)
// Γ,∆,τr ,loop ⊢While(g,body) : ok(false) 
bool While::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    auto guardType = guard->check(gamma, delta);
    if (!typeEq(guardType, std::make_shared<IntType>())) {
         throw TypeError("non-int type " + guardType->toString() + " for while guard '" + guard->toString() + "'");
    }
    // Check body with inLoop = true
    body->check(gamma, delta, returnType, true); // Body return status doesn't matter for the while loop itself
    return false; // While loop never definitely returns
}

// Γ,∆ ⊢e : τ eq(τ,τr)
// Γ,∆,τr ,loop ⊢Return(e) : ok(true)
bool Return::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    if (exp.has_value()) {
        auto expType = (*exp)->check(gamma, delta);
        if (!typeEq(expType, returnType)) {
             throw TypeError("incompatible return type " + expType->toString() + " for 'return " + (*exp)->toString() + "', should be " + returnType->toString());
        }
    } else {
        // Handle void return. Let's assume non-int return types aren't allowed yet based on main's spec
         if (!typeEq(returnType, std::make_shared<IntType>())) { // Placeholder check - adjust if void is added
             throw TypeError("missing return expression for non-int function type " + returnType->toString());
         }
         // If we allow void, check if returnType is void here
         throw TypeError("return statement requires an expression in this function"); // Assuming non-void for now
    }
    return true; // Return statement always definitely returns
}

// loop= true
// Γ,∆,τr ,loop ⊢Break : ok(false)
bool Break::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    if (!inLoop) {
        throw TypeError("break outside loop");
    }
    return false; // Break never definitely returns
}

// loop= true
// Γ,∆,τr ,loop ⊢Continue : ok(false)
bool Continue::check(const Gamma& gamma, const Delta& delta, const std::shared_ptr<Type>& returnType, bool inLoop) const {
    // Rule CONTINUE
    if (!inLoop) {
        throw TypeError("continue outside loop");
    }
    return false; // Continue never definitely returns
}


// --- Top-Level Check Implementations ---

// |flds|>0 ∀Decl(name,τ) ∈flds.[τ ̸∈{nil,struct( ),fn(, )}]
// Γ,∆ ⊢Struct(name,flds) : ok
void StructDef::check(const Gamma& gamma, const Delta& delta) const {
    if (fields.empty()) {
        throw TypeError("empty struct " + name);
    }
    std::set<std::string> fieldNames; // To check for duplicate field names locally
    for (const auto& field : fields) {
        // Check field type validity
        if (dynamic_cast<NilType*>(field.type.get()) || dynamic_cast<StructType*>(field.type.get()) || dynamic_cast<FnType*>(field.type.get())) {
             throw TypeError("invalid type " + field.type->toString() + " for struct field " + name + "::" + field.name);
        }
         // Check for duplicate field names within this struct
        if (!fieldNames.insert(field.name).second) {
             throw TypeError("Duplicate field name '" + field.name + "' in struct '" + name + "'");
        }
    }
}

// Γ′ = Γ + (prms ∪locals) Γ′, ∆,τr ,false ⊢stmts : ok(true) ∀(Decl(name,τ),e) ∈(prms ∪locals).[τ ̸∈{nil,struct( ),fn(, )}]
// Γ,∆ ⊢Function(name,prms,τr,locals,stmts) : ok
void FunctionDef::check(const Gamma& gamma, const Delta& delta) const {
    Gamma localGamma = gamma; // Copy global gamma
    std::set<std::string> localNames; // Check param/local duplicates

    // Add parameters to localGamma and check types/duplicates
    for(const auto& p : params) {
        if (dynamic_cast<NilType*>(p.type.get()) || dynamic_cast<StructType*>(p.type.get()) || dynamic_cast<FnType*>(p.type.get())) {
             throw TypeError("invalid type " + p.type->toString() + " for variable " + p.name + " in function " + name);
        }
        if (!localNames.insert(p.name).second) {
            throw TypeError("Duplicate parameter/local name '" + p.name + "' in function '" + name + "'");
        }
        localGamma[p.name] = p.type;
    }
     // Add locals to localGamma and check types/duplicates
    for(const auto& l : locals) {
        if (dynamic_cast<NilType*>(l.type.get()) || dynamic_cast<StructType*>(l.type.get()) || dynamic_cast<FnType*>(l.type.get())) {
             throw TypeError("invalid type " + l.type->toString() + " for variable " + l.name + " in function " + name);
        }
         if (!localNames.insert(l.name).second) {
            throw TypeError("Duplicate parameter/local name '" + l.name + "' in function '" + name + "'");
        }
         localGamma[l.name] = l.type;
    }

    // Check if body exists (rule [stmts: ok(true)] means body must exist and return)
    if (!body) {
        throw TypeError("function " + name + " has an empty body");
    }
    // check if the Stmts node is empty
     if (auto stmtsPtr = dynamic_cast<Stmts*>(body.get())) {
         if (stmtsPtr->statements.empty()) {
             throw TypeError("function " + name + " has an empty body");
         }
     } else {
         // This implies the body isn't even a Stmts node, which is likely a parsing/AST build error
          throw TypeError("function " + name + " has an invalid body structure (expected Stmts)");
     }

    // Check body with inLoop = false. Must definitely return (ok(true)).
    bool definitelyReturns = body->check(localGamma, delta, rettype, false);

    if (!definitelyReturns) {
        throw TypeError("function " + name + " may not execute a return");
    }
}

// Γ = construct-gamma(externs,funcs) ∆ = construct-delta(structs) ∃f ∈funcs.[f.name = main ∧f.prms= ⟨⟩∧f.rettyp= int] ∀s∈structs.[Γ,∆ ⊢s: ok] 
// ∀f ∈funcs.[Γ,∆ ⊢f : ok]
// ⊢Program(structs,externs,funcs) : ok
void Program::check() const {
    // Check for duplicate names among structs, externs, functions first
    std::set<std::string> topLevelNames;
    for (const auto& s : structs) {
        if (!topLevelNames.insert(s->name).second) throw TypeError("Duplicate name: " + s->name);
    }
     for (const auto& e : externs) {
        if (!topLevelNames.insert(e.name).second) throw TypeError("Duplicate name: " + e.name);
    }
     for (const auto& f : functions) {
         // Allow 'main' to exist even if not in the set yet, checked later
        if (f->name != "main" && !topLevelNames.insert(f->name).second) throw TypeError("Duplicate name: " + f->name);
    }
    // Check main again just in case it conflicts with a struct/extern
     if(topLevelNames.count("main") && std::find_if(functions.begin(), functions.end(), [](const auto& f){ return f->name == "main"; }) != functions.end()){
         // This case is tricky - technically allowed by the set check if main wasn't inserted yet.
         // construct_gamma will likely catch it too. Add a specific check?
         throw TypeError("Duplicate name: main");
     }

    Gamma initial_gamma = construct_gamma(externs, functions);
    Delta initial_delta = construct_delta(structs);

    bool mainFound = false;
    for (const auto& func : functions) {
        if (func->name == "main") {
            // Check signature: fn((), int)
            if (func->params.empty() && typeEq(func->rettype, std::make_shared<IntType>())) {
                mainFound = true;
            } else {
                 throw TypeError("function 'main' exists but has wrong type, should be '() -> int'");
            }
        }
    }

    if (!mainFound) {
        throw TypeError("no 'main' function with type '() -> int' exists");
    }

    // Check structs (Rule STRUCT applied via StructDef::check)
    for (const auto& s : structs) {
        s->check(initial_gamma, initial_delta);
    }

     // Check functions (Rule FUNCTION applied via FunctionDef::check)
    for (const auto& f : functions) {
        f->check(initial_gamma, initial_delta);
    }
}

// JSON to AST Conversion Implementations

// Parses type representations from JSON
std::shared_ptr<Type> buildType(const nlohmann::json& j) {
    if (j.is_string()) {
        const std::string& kind = j.get<std::string>();
        if (kind == "Int") return std::make_shared<IntType>();
        if (kind == "Nil") return std::make_shared<NilType>();
        // Add other simple string types if Cflat has them (e.g., "Bool"?)
        throw std::runtime_error("Unknown simple type string: " + kind);
    }

    if (j.is_object()) {
        // Use .value("key", default) for potentially missing keys if needed later
        if (j.contains("Struct")) { // Assuming {"Struct": "name"}
             return std::make_shared<StructType>(j.at("Struct").get<std::string>());
        }
         if (j.contains("Ptr")) { // Assuming {"Ptr": Type}
             return std::make_shared<PtrType>(buildType(j.at("Ptr")));
        }
         if (j.contains("Array")) { // Assuming {"Array": Type}
             return std::make_shared<ArrayType>(buildType(j.at("Array")));
        }
         if (j.contains("Fn")) { // Assuming {"Fn": [ [ParamTypes], ReturnType ]}
            const auto& fn_sig = j.at("Fn");
            if (!fn_sig.is_array() || fn_sig.size() != 2 || !fn_sig[0].is_array()) {
                 throw std::runtime_error("Invalid JSON for Fn type signature");
            }
            std::vector<std::shared_ptr<Type>> params;
            for (const auto& p : fn_sig[0]) {
                params.push_back(buildType(p));
            }
            return std::make_shared<FnType>(std::move(params), buildType(fn_sig[1]));
        }
         // Handle object representation for simple types if they exist, e.g. {"kind":"Int"}
         if(j.contains("kind")) {
             const std::string& kind = j.at("kind").get<std::string>();
             if (kind == "Int") return std::make_shared<IntType>();
             if (kind == "Nil") return std::make_shared<NilType>();
             // Add others if necessary
         }
    }

    throw std::runtime_error("Invalid JSON for Type: " + j.dump());
}

// Parses Place representations (Id, Deref, ArrayAccess, FieldAccess) from JSON.
std::unique_ptr<Place> buildPlace(const nlohmann::json& j) {
    if (!j.is_object() || j.empty()) {
        throw std::runtime_error("Invalid JSON for Place: Must be non-empty object");
    }
    // The key determines the kind
    const auto& key = j.begin().key();
    const auto& value = j.begin().value();

     if (key == "Id") { // {"Id": "name"}
         return std::make_unique<Id>(value.get<std::string>());
     }
     if (key == "Deref") { // {"Deref": Exp}
         return std::make_unique<Deref>(buildExp(value));
     }
     if (key == "ArrayAccess") { // {"ArrayAccess": {"array": Exp, "idx": Exp}}
         if (!value.is_object() || !value.contains("array") || !value.contains("idx")) {
              throw std::runtime_error("Invalid JSON for ArrayAccess content");
         }
         return std::make_unique<ArrayAccess>(buildExp(value.at("array")), buildExp(value.at("idx")));
     }
     if (key == "FieldAccess") { // {"FieldAccess": {"ptr": Exp, "field": "name"}}
         if (!value.is_object() || !value.contains("ptr") || !value.contains("field")) {
              throw std::runtime_error("Invalid JSON for FieldAccess content");
         }
         return std::make_unique<FieldAccess>(buildExp(value.at("ptr")), value.at("field").get<std::string>());
     }

     throw std::runtime_error("JSON node is not a valid Place kind: " + key);
}

// Parses Expression representations from JSON.
std::unique_ptr<Exp> buildExp(const nlohmann::json& j) {
    if (!j.is_object() || j.empty()) {
        // Allow Nil if represented differently, check specific case
        if (j.is_string() && j.get<std::string>() == "Nil") { // Check if Nil is just a string
            return std::make_unique<NilExp>();
        }
         if (j.is_object() && j.contains("kind") && j.at("kind") == "Nil") { // Check if Nil uses kind
             return std::make_unique<NilExp>();
         }
        throw std::runtime_error("Invalid JSON for Exp: Must be non-empty object or known literal");
    }

    const auto& key = j.begin().key();
    const auto& value = j.begin().value();

    // Places wrapped in Val
    // Check if the key indicates a Place kind
    if (key == "Id" || key == "Deref" || key == "ArrayAccess" || key == "FieldAccess") {
        // Need to reconstruct the original object for buildPlace
        nlohmann::json place_obj;
        place_obj[key] = value;
        return std::make_unique<Val>(buildPlace(place_obj));
    }
    // Direct Expressions
    if (key == "Num") { // {"Num": number}
        return std::make_unique<Num>(value.get<long long>());
    }
    if (key == "Nil") { // {"Nil": null} or possibly just {"kind":"Nil"} handled above
        return std::make_unique<NilExp>();
    }
    if (key == "Select") { // {"Select": {"guard": Exp, "tt": Exp, "ff": Exp}}
         if (!value.is_object() || !value.contains("guard") || !value.contains("tt") || !value.contains("ff")) {
              throw std::runtime_error("Invalid JSON for Select content");
         }
        return std::make_unique<Select>(buildExp(value.at("guard")), buildExp(value.at("tt")), buildExp(value.at("ff")));
    }
    if (key == "UnOp") { // {"UnOp": {"op": "Neg"|"Not", "exp": Exp}}
        if (!value.is_object() || !value.contains("op") || !value.contains("exp")) {
             throw std::runtime_error("Invalid JSON for UnOp content");
        }
         UnaryOp op;
         std::string opStr = value.at("op").get<std::string>();
         if(opStr == "Neg") op = UnaryOp::Neg;
         else if(opStr == "Not") op = UnaryOp::Not;
         else throw std::runtime_error("Unknown unary operator: " + opStr);
         return std::make_unique<UnOp>(op, buildExp(value.at("exp")));
    }
    if (key == "BinOp") { // {"BinOp": {"op": "Add"|..., "left": Exp, "right": Exp}}
        if (!value.is_object() || !value.contains("op") || !value.contains("left") || !value.contains("right")) {
             throw std::runtime_error("Invalid JSON for BinOp content");
        }
         BinaryOp op;
         std::string opStr = value.at("op").get<std::string>();
         // Simplified - Add all ops
         if(opStr == "Add") op = BinaryOp::Add; else if(opStr == "Sub") op = BinaryOp::Sub;
         else if(opStr == "Mul") op = BinaryOp::Mul; else if(opStr == "Div") op = BinaryOp::Div;
         else if(opStr == "And") op = BinaryOp::And; else if(opStr == "Or") op = BinaryOp::Or;
         else if(opStr == "Eq") op = BinaryOp::Eq; else if(opStr == "NotEq") op = BinaryOp::NotEq;
         else if(opStr == "Lt") op = BinaryOp::Lt; else if(opStr == "Lte") op = BinaryOp::Lte;
         else if(opStr == "Gt") op = BinaryOp::Gt; else if(opStr == "Gte") op = BinaryOp::Gte;
         else throw std::runtime_error("Unknown binary operator: " + opStr);
         return std::make_unique<BinOp>(op, buildExp(value.at("left")), buildExp(value.at("right")));
    }
    if (key == "NewSingle") { // {"NewSingle": Type}
         return std::make_unique<NewSingle>(buildType(value));
    }
     if (key == "NewArray") { // {"NewArray": {"type": Type, "size": Exp}}
         if (!value.is_object() || !value.contains("type") || !value.contains("size")) {
             throw std::runtime_error("Invalid JSON for NewArray content");
         }
         return std::make_unique<NewArray>(buildType(value.at("type")), buildExp(value.at("size")));
    }
     if (key == "CallExp") { // {"CallExp": FunCall}
         return std::make_unique<CallExp>(buildFunCall(value));
     }
     if (key == "Val") { // Handle explicit Val if it appears: {"Val": Place}
         return std::make_unique<Val>(buildPlace(value));
     }

    throw std::runtime_error("Unknown/Unhandled expression kind: " + key + " with value " + value.dump());
}

// Parses FunCall representation from JSON.
std::unique_ptr<FunCall> buildFunCall(const nlohmann::json& j) {
    // Assuming format {"callee": Exp, "args": [Exp, ...]}
    if (!j.is_object() || !j.contains("callee") || !j.contains("args") || !j.at("args").is_array()) {
         throw std::runtime_error("Invalid JSON for FunCall");
    }
     std::vector<std::unique_ptr<Exp>> args;
     for(const auto& arg : j.at("args")) {
         args.push_back(buildExp(arg));
     }
    return std::make_unique<FunCall>(buildExp(j.at("callee")), std::move(args));
}

// Parses Statement representations from JSON.
std::unique_ptr<Stmt> buildStmt(const nlohmann::json& j) {
    if (!j.is_object() || j.empty()) {
        throw std::runtime_error("Invalid JSON for Stmt: Must be non-empty object");
    }
    const auto& key = j.begin().key();
    const auto& value = j.begin().value();

    if (key == "Assign") { // {"Assign": [Place, Exp]}
        if (!value.is_array() || value.size() != 2) {
             throw std::runtime_error("Invalid JSON for Assign content");
        }
        return std::make_unique<Assign>(buildPlace(value[0]), buildExp(value[1]));
    }
    if (key == "Call") { // {"Call": FunCall} - Assuming 'Call' implies CallStmt
         return std::make_unique<CallStmt>(buildFunCall(value));
    }
     if (key == "If") { // {"If": {"guard": Exp, "tt": Stmt, "ff": Stmt|null}}
        if (!value.is_object() || !value.contains("guard") || !value.contains("tt")) {
             throw std::runtime_error("Invalid JSON for If content");
        }
        std::optional<std::unique_ptr<Stmt>> ff = std::nullopt;
        // Use .value("key", default) for optional field
        nlohmann::json ff_json = value.value("ff", nlohmann::json()); // Get "ff" or empty object
        if (!ff_json.is_null() && !(ff_json.is_object() && ff_json.empty())) { // Check if it's not null/empty
             ff = buildStmt(ff_json);
        }
        return std::make_unique<If>(buildExp(value.at("guard")), buildStmt(value.at("tt")), std::move(ff));
    }
     if (key == "While") { // {"While": {"guard": Exp, "body": Stmt}}
         if (!value.is_object() || !value.contains("guard") || !value.contains("body")) {
             throw std::runtime_error("Invalid JSON for While content");
         }
        return std::make_unique<While>(buildExp(value.at("guard")), buildStmt(value.at("body")));
    }
    if (key == "Return") { // {"Return": Exp | null}
         std::optional<std::unique_ptr<Exp>> exp = std::nullopt;
         if (!value.is_null()) { // Handle optional return expression
             exp = buildExp(value);
         }
        return std::make_unique<Return>(std::move(exp));
    }
    if (key == "Break") { // {"Break": null}
        return std::make_unique<Break>();
    }
    if (key == "Continue") { // {"Continue": null}
        return std::make_unique<Continue>();
    }
    // Handle Stmts node explicitly if it can appear nested (e.g., in If/While without Stmts wrapper)
    // This depends on how the JSON is generated. If If/While bodies are *always* arrays of statements,
    // we need to wrap them in a Stmts node here. Assuming for now buildStmt is called on the outer object.
    // If the JSON for If/While looks like {"If": {..., "tt": [Stmt, Stmt]}}, then buildStmt needs adjustment.
    // **Correction based on If/While AST definition:** Your AST expects unique_ptr<Stmt>, so the build
    // function *should* build a Stmts node if the JSON gives an array.
     if (key == "Stmts") { // Handle explicit Stmts node {"Stmts": [Stmt, ...]}
         auto node = std::make_unique<Stmts>();
         if (!value.is_array()) throw std::runtime_error("Invalid JSON for Stmts content");
         for(const auto& s : value) {
             node->statements.push_back(buildStmt(s));
         }
         return node;
     }


    throw std::runtime_error("Unknown statement kind: " + key);
}

// Parses Decl representations (used in params, locals, fields) from JSON.
Decl buildDecl(const nlohmann::json& j) {
     // Assuming {"name": "...", "typ": Type}
    if (!j.is_object() || !j.contains("name") || !j.contains("typ")) {
        throw std::runtime_error("Invalid JSON for Decl");
    }
    return {j.at("name").get<std::string>(), buildType(j.at("typ"))};
}

// Parses FunctionDef representations from JSON.
std::unique_ptr<FunctionDef> buildFunctionDef(const nlohmann::json& j) {
    if (!j.is_object() || !j.contains("name") || !j.contains("prms") || !j.contains("rettyp") || !j.contains("locals") || !j.contains("stmts")) {
         throw std::runtime_error("Invalid JSON for Function definition");
    }
    auto func = std::make_unique<FunctionDef>();
    func->name = j.at("name").get<std::string>();
    func->rettype = buildType(j.at("rettyp"));
    for (const auto& p : j.at("prms")) {
        func->params.push_back(buildDecl(p));
    }
    for (const auto& l : j.at("locals")) {
        func->locals.push_back(buildDecl(l));
    }
    // IMPORTANT: Wrap the array of statements from JSON into a single Stmts node for the body
    auto bodyStmts = std::make_unique<Stmts>();
    if (!j.at("stmts").is_array()){
         throw std::runtime_error("Invalid JSON: function 'stmts' must be an array");
    }
    for(const auto& s : j.at("stmts")) {
        bodyStmts->statements.push_back(buildStmt(s));
    }
    func->body = std::move(bodyStmts);

    return func;
}

// Parses StructDef representations from JSON.
std::unique_ptr<StructDef> buildStructDef(const nlohmann::json& j) {
    // Assuming {"name": "...", "fields": [Decl, ...]}
    if (!j.is_object() || !j.contains("name") || !j.contains("fields") || !j.at("fields").is_array()) {
         throw std::runtime_error("Invalid JSON for Struct definition");
    }
     auto s = std::make_unique<StructDef>();
     s->name = j.at("name").get<std::string>();
     for (const auto& f : j.at("fields")) {
         s->fields.push_back(buildDecl(f));
     }
     return s;
}

// Parses Extern representations from JSON.
Extern buildExtern(const nlohmann::json& j) {
    // Assuming {"name": "...", "params": [Type, ...], "rettyp": Type}
    // Note: JSON key might be "prms" like in FunctionDef, adjust if necessary. Assuming "params" for now.
     if (!j.is_object() || !j.contains("name") || !j.contains("params") || !j.contains("rettyp") || !j.at("params").is_array()) {
         throw std::runtime_error("Invalid JSON for Extern definition");
    }
    Extern e;
    e.name = j.at("name").get<std::string>();
    e.rettype = buildType(j.at("rettype"));
     for (const auto& p_type : j.at("params")) { // Externs just list types
        e.param_types.push_back(buildType(p_type));
    }
    return e;
}

// Parses the top-level Program object from JSON.
std::unique_ptr<Program> buildProgram(const nlohmann::json& j) {
    // Assuming {"structs": [...], "externs": [...], "functions": [...]}
    if (!j.is_object() || !j.contains("structs") || !j.contains("externs") || !j.contains("functions")) {
         throw std::runtime_error("Invalid JSON for Program root object");
    }
    auto prog = std::make_unique<Program>();
    for (const auto& s : j.at("structs")) {
        prog->structs.push_back(buildStructDef(s));
    }
    for (const auto& e : j.at("externs")) {
        prog->externs.push_back(buildExtern(e));
    }
    for (const auto& f : j.at("functions")) {
        prog->functions.push_back(buildFunctionDef(f));
    }
    return prog;
}

// --- Environment Construction Implementations ---

Gamma construct_gamma(const std::vector<Extern>& externs, const std::vector<std::unique_ptr<FunctionDef>>& functions) {
    Gamma gamma;
    // Add externs (type fn)
    for (const auto& ext : externs) {
        // Basic duplicate check (assuming no name conflicts guaranteed by parser as per spec)
        gamma[ext.name] = std::make_shared<FnType>(ext.param_types, ext.rettype);
    }
    // Add internal functions (type ptr(fn)) - except main
    for (const auto& func : functions) {
        if (func->name != "main") {
            std::vector<std::shared_ptr<Type>> paramTypes;
            for(const auto& p : func->params) {
                paramTypes.push_back(p.type);
            }
            auto fnType = std::make_shared<FnType>(std::move(paramTypes), func->rettype);
            gamma[func->name] = std::make_shared<PtrType>(fnType);
        }
    }
    return gamma;
}

Delta construct_delta(const std::vector<std::unique_ptr<StructDef>>& structs) {
    Delta delta;
    for (const auto& s : structs) {
         // Basic duplicate check (assuming no name conflicts guaranteed by parser as per spec)
        std::unordered_map<std::string, std::shared_ptr<Type>> fields;
        for (const auto& f : s->fields) {
            // Basic duplicate check (assuming no name conflicts guaranteed by parser as per spec)
            fields[f.name] = f.type;
        }
        delta[s->name] = std::move(fields);
    }
    return delta;
}