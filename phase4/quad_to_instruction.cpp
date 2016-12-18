#include "quad_to_instruction.h"

void convert::generate(vector<quad*>& q, uint globalCount) {
    this->quads = q;
    this->globalCount = globalCount;
    for (uint i = 0; i < q.size(); ++i) {
        currentProcessedQuad = i;
        (*this.*generators[q[i]->op])(q[i]);
    }

    patch_incomplete_jumps();
    writeToFile();
}

void convert::generate_op(vmopcode op, quad *q) {
    instruction* t = new instruction();
    t->opcode = op;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_operand(q->arg2, &t->arg2);
    make_operand(q->result, &t->result);
    q->taddress = nextInstructionLabel();
    emit(t);
}

void convert::generate_relational(vmopcode op, quad *q){
    instruction* t = new instruction();
    t->opcode = op;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_operand(q->arg2, &t->arg2);

    t->result.type = label_a;
    if (q->label < currentProcessedQuad){
        t->result.value = quads[q->label]->taddress;
    }
    else{
        add_incomplete_jump(nextInstructionLabel(), q->label);
    }
    q->taddress = nextInstructionLabel();
    emit(t);
}

void convert::generate_ADD(quad* q) {
    generate_op(add_v, q);
}

void convert::generate_SUB(quad* q) {
    generate_op(sub_v, q);
}

void convert::generate_MUL(quad* q) {
    generate_op(mul_v, q);
}

void convert::generate_DIV(quad* q) {
    generate_op(div_v, q);
}

void convert::generate_MOD(quad* q) {
    generate_op(mod_v, q);
}

void convert::generate_UMINUS(quad *q) {
    instruction* t = new instruction();
    t->opcode = mul_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_numberoperand(&t->arg2, -1);
    make_operand(q->result, &t->result);
    q->taddress = nextInstructionLabel();
    emit(t);
}

void convert::generate_NEWTABLE(quad* q) {
    generate_op(newtable_v, q);
}

void convert::generate_TABLEGETELEM(quad* q) {
    generate_op(tablegetelem_v, q);
}

void convert::generate_TABLESETELEM(quad* q) {
    generate_op(tablesetelem_v, q);
}

void convert::generate_ASSIGN(quad* q) {
    generate_op(assign_v, q);
}

void convert::generate_NOP(quad* q) {
    instruction* t = new instruction();
    t->opcode = nop_v;
    emit(t);
}

void convert::generate_JUMP(quad* q) {
    generate_relational(jump_v, q);
}

void convert::generate_IF_EQ(quad* q) {
    generate_relational(jeq_v, q);
}

void convert::generate_IF_NOTEQ(quad* q) {
    generate_relational(jne_v, q);
}

void convert::generate_IF_GREATER(quad* q) {
    generate_relational(jgt_v, q);
}

void convert::generate_IF_GREATEREQ(quad* q) {
    generate_relational(jge_v, q);
}

void convert::generate_IF_LESS(quad* q) {
    generate_relational(jlt_v, q);
}

void convert::generate_IF_LESSEQ(quad* q) {
    generate_relational(jle_v, q);
}

void convert::generate_NOT(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->srcLine = q->line;
    t->opcode = jeq_v;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, false);
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 3;
    emit(t);
    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, false);
    //reset_operand(&t.arg2);
    make_operand(q->result, &t->result);
    emit(t);
    t = new instruction();
    t->opcode = jump_v;
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 2;
    emit(t);
    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, true);
    make_operand(q->result, &t->result);
    emit(t);

}

void convert::generate_OR(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->srcLine = q->line;
    t->opcode = jeq_v;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, true);
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 4;
    emit(t);

    t = new instruction();
    t->opcode = jeq_v;
    make_operand(q->arg2, &t->arg1);
    make_booloperand(&t->arg2, true);
    t->result.value = nextInstructionLabel() + 3;
    emit(t);

    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, false);
    make_operand(q->result, &t->result);
    emit(t);

    t = new instruction();
    t->opcode = jump_v;
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 2;
    emit(t);

    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, true);
    make_operand(q->result, &t->result);
    emit(t);
}

void convert::generate_AND(quad* q){
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->srcLine = q->line;
    t->opcode = jeq_v;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, false);
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 4;
    emit(t);

    t = new instruction();
    t->opcode = jeq_v;
    make_operand(q->arg2, &t->arg1);
    make_booloperand(&t->arg2, false);
    t->result.value = nextInstructionLabel() + 3;
    emit(t);

    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, true);
    make_operand(q->result, &t->result);
    emit(t);

    t = new instruction();
    t->opcode = jump_v;
    t->result.type = label_a;
    t->result.value = nextInstructionLabel() + 2;
    emit(t);

    t = new instruction();
    t->opcode = assign_v;
    make_booloperand(&t->arg1, false);
    make_operand(q->result, &t->result);
    emit(t);
}

void convert::generate_PARAM(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->opcode = pusharg_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    emit(t);
}

void convert::generate_CALL(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->opcode = call_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    emit(t);
}

void convert::generate_GETRETVAL(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->opcode = assign_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    make_retvaloperand(&t->arg1);
    emit(t);
}

void convert::generate_FUNCSTART(quad* q) {
    SymbolEntry *f = q->result->sym;
    f->setAddress(nextInstructionLabel());
    q->taddress = nextInstructionLabel();

    userfuncs_newfunc(f);
    vector<int> a;
    funcstack.push_back(a);

    instruction* t = new instruction();
    t->opcode = funcenter_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    emit(t);
}

void convert::generate_RETURN(quad* q) {
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->opcode = assign_v;
    t->srcLine = q->line;
    make_retvaloperand(&t->result);
    make_operand(q->result, &t->arg1);
    emit(t);

    funcstack.back().push_back(nextInstructionLabel());

    t = new instruction();
    t->opcode = jump_v;
    t->result.type = label_a;
    emit(t);
}

void convert::generate_FUNCEND(quad* q) {
    vector<int> a = funcstack.back();
    funcstack.pop_back();
    //TODO backpatch
    backpatch_t(a);
    q->taddress = nextInstructionLabel();
    instruction *t = new instruction();
    t->opcode = funcexit_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    emit(t);
}

void convert::make_operand(expr *e, vmarg *arg) {
    if (e == NULL) return;
    switch (e->type){
        case var_e:
        case tableitem_e:
        case arithexpr_e:
        case boolexpr_e:
        case assignexpr_e:
        case newtable_e:
            arg->value = (uint) e->sym->getOffset();
            switch (e->sym->getSpace()){
                case PROGRAM_VAR:
                    arg->type = global_a;
                    break;
                case FUNCTION_LOCAL:
                    arg->type = local_a;
                    break;
                case FORMAL_ARG:
                    arg->type = formal_a;
                    break;
            }
            break;

        case constbool_e:
            arg->value = (uint) e->boolConst;
            arg->type = bool_a;
            break;
        case conststring_e:
            arg->value = consts_newstring(e->strConst);
            arg->type = string_a;
            break;
        case constnum_e:
            arg->value = consts_newnumber(e->numConst);
            arg->type = number_a;
            break;
        case nil_e:
            arg->type = nil_a;
            break;
        case programfunc_e:
            arg->type = userfunc_a;
            arg->value = e->sym->getAddress();
            break;
        case libraryfunc_e:
            arg->type = libfunc_a;
            arg->value = libfuncs_newused(e->sym->getName());
            break;
        default:
            break;
    }
}

void convert::make_numberoperand(vmarg* arg, double val){
    arg->value = consts_newnumber(val);
    arg->type = number_a;
}

void convert::make_booloperand(vmarg* arg, bool val){
    arg->value = (uint) val;
    arg->type = bool_a;
}

void convert::make_retvaloperand(vmarg* arg){
    arg->type = retval_a;
}

void convert::patch_incomplete_jumps() {
    //TODO ?
    for (int i = 0; i < ijs.size(); ++i) {
        if(ijs[i].iaddress == quads.size()){
            instructions[ijs[i].instrNo]->result.value = nextInstructionLabel();
        }
        else{
            instructions[ijs[i].instrNo]->result.value = quads[ijs[i].iaddress]->taddress;
        }
    }
}

convert::convert() {
    int i = 0;
    generators[i++] = &convert::generate_ASSIGN;
    generators[i++] = &convert::generate_ADD;
    generators[i++] = &convert::generate_SUB;
    generators[i++] = &convert::generate_MUL;
    generators[i++] = &convert::generate_DIV;
    generators[i++] = &convert::generate_MOD;
    generators[i++] = &convert::generate_UMINUS;
    generators[i++] = &convert::generate_AND;
    generators[i++] = &convert::generate_OR;
    generators[i++] = &convert::generate_NOT;
    generators[i++] = &convert::generate_IF_EQ;
    generators[i++] = &convert::generate_IF_NOTEQ;
    generators[i++] = &convert::generate_IF_LESSEQ;
    generators[i++] = &convert::generate_IF_GREATEREQ;
    generators[i++] = &convert::generate_IF_LESS;
    generators[i++] = &convert::generate_IF_GREATER;
    generators[i++] = &convert::generate_JUMP;
    generators[i++] = &convert::generate_CALL;
    generators[i++] = &convert::generate_PARAM;
    generators[i++] = &convert::generate_RETURN;
    generators[i++] = &convert::generate_GETRETVAL;
    generators[i++] = &convert::generate_FUNCSTART;
    generators[i++] = &convert::generate_FUNCEND;
    generators[i++] = &convert::generate_NEWTABLE;
    generators[i++] = &convert::generate_TABLEGETELEM;
    generators[i++] = &convert::generate_TABLESETELEM;
    generators[i++] = &convert::generate_NOP;
}

uint convert::consts_newstring(string s) {
    uint r = (uint)const_strings.size();
    for (int i = 0; i < r; i++)
    {
        if (const_strings[i] == s) return i;
    }
    const_strings.push_back(s);
    return r;
}

uint convert::consts_newnumber(double n) {
    uint r = (uint) const_nums.size();
    for (int i = 0; i < r; i++)
    {
        if (const_nums[i] == n) return i;
    }
    const_nums.push_back(n);
    return r;
}

uint convert::libfuncs_newused(string s) {
    uint r = (uint) const_libfuncs.size();
    for (int i = 0; i < r; i++)
    {
        if (const_libfuncs[i] == s) return i;
    }
    const_libfuncs.push_back(s);
    return r;
}
uint convert::userfuncs_newfunc(SymbolEntry *sym) {
    uint r = (uint) const_userfuncs.size();
    userfunc u;
    u.id = sym->getName();
    u.address = sym->getAddress();
    u.localSize = sym->getTotalLocals();
    const_userfuncs.push_back(u);
    return r;
}

void convert::writeToFile() {
    ofstream fileb ("binary", ios::out|ios::binary);
    ofstream file ("instructions.txt", ios::out);
    if (file.is_open() && fileb.is_open())
    {
        uint total = 0;
        writeNum(340200501, fileb);

        file << "Constants" << endl << endl;

        // write strings
        file << "Strings" << endl;
        total = const_strings.size();
        writeNum(total, fileb);
        for(int i = 0; i < total; i++){
            writeString(const_strings[i], fileb);
            file << i << ": " << const_strings[i] << endl;
        }
        file << endl;

        // read numbers
        file << "Numbers" << endl;
        total = const_nums.size();
        writeNum(total, fileb);
        for(int i = 0; i < total; i++){
            writeDouble(const_nums[i], fileb);
            file << i << ": " << const_nums[i] << endl;
        }
        file << endl;

        // read libfunctions
        file << "LibFunctions" << endl;
        total = const_libfuncs.size();
        writeNum(total, fileb);
        for(int i = 0; i < total; i++){
            writeString(const_libfuncs[i], fileb);
            file << i << ": " << const_libfuncs[i] << endl;
        }
        file << endl;

        // read user functions
        file << "User Functions" << endl;
        total = const_userfuncs.size();
        writeNum(total, fileb);
        for(int i = 0; i < total; i++){
            writeNum(const_userfuncs[i].address, fileb);
            writeNum(const_userfuncs[i].localSize, fileb);
            writeString(const_userfuncs[i].id, fileb);

            file << i << ": " << const_userfuncs[i].id << " " << const_userfuncs[i].address << " " << const_userfuncs[i].localSize << endl;
        }
        file << endl;

        // read code
        file << "Instructions" << endl;
        total = instructions.size();
        writeNum(total, fileb);

        file << setw(3) << "---" << setw(16) << "OP" << setw(6) << "|"
        << setw(16) << "RESULT" << setw(7) << "|"
        << setw(13) << "ARG1" << setw(10) << "|"
        << setw(13) << "ARG2" << setw(10) << "|"
        << setw(9) << "LINE" << setw(6) << "|"
        << endl;
        for (int i = 0; i < total; i++){
            char* r = new char[16];
            instruction *s  = instructions[i];

            r[0] = s->opcode;
            r[1] = s->result.type;
            *(uint*)(r + 2) = s->result.value;
            //s.operand2 = operand();
            r[6] = s->arg1.type;
            *(uint*)(r + 7) = s->arg1.value;
            //s.result = operand();
            r[11] = s->arg2.type;
            *(uint*)(r + 12) = s->arg2.value;

            file << setw(3) << i << setw(16) << vmopcode_names[instructions[i]->opcode] << setw(6) << "|";
            if (instructions[i]->result.type == undef_a) file << setw(20) << "-" << setw(3) << "|";
            else file << setw(12) << vmarg_names[instructions[i]->result.type] << setw(6) << "|val:" << setw(2) << instructions[i]->result.value << setw(3) << "|";
            if (instructions[i]->arg1.type == undef_a) file << setw(20) << "-" << setw(3) << "|";
            else file << setw(12) << vmarg_names[instructions[i]->arg1.type] << setw(6) << "|val:" << setw(2) << instructions[i]->arg1.value << setw(3) << "|";
            if (instructions[i]->arg2.type == undef_a) file << setw(20) << "-" << setw(3) << "|";
            else file << setw(12) << vmarg_names[instructions[i]->arg2.type] << setw(6) << "|val:" << setw(2) << instructions[i]->arg2.value << setw(3) << "|";
            file << setw(7) << instructions[i]->srcLine << setw(8) << "|"
            << endl;

            fileb.write(r, 16);
        }
        file << endl;

        file << "Global Count: " << globalCount << endl;
        writeNum(globalCount, fileb);

        file.close();
        fileb.close();
    }
    else cout << "Unable to open files" << endl;
}
void convert::writeDouble(double num, ofstream& file){
    file.write((char*)&num, 8);
}

void convert::writeNum(uint num, ofstream& file){
    file.write((char*)&num, 4);
}

void convert::writeString(string s, ofstream& file){
    int size = s.length();
    file.write((char*)&size, 4);
    file.write(s.c_str(), size);
}

void convert::backpatch_t(vector<int>& a) {
    for (int i = 0; i < a.size(); i++) {
        instructions[a[i]]->result.value = nextInstructionLabel();
    }
    return;
}
