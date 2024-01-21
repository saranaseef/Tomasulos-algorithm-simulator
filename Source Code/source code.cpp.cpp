#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <iomanip> 
using namespace std;

struct ReservationStation {
    bool isBusy;
    string operation;
    int Vj;
    int Vk;
    pair<string, int> Qj;
    pair<string, int> Qk;
    int A;
    int exe_cycles;
    int counter;
    string station_name;
    int station_num;
    int inst_num;
    int stage;
    bool branch_predict;
    int tablePosition;

};

vector<int> parseInstruction(string instruction) {
    vector<int> parsedOutput;

    string opCode, destReg, srcReg1, srcReg2;

    // Extract operation code
    size_t spacePos = instruction.find(' ');
    opCode = instruction.substr(0, spacePos);
    instruction.erase(0, spacePos != string::npos ? spacePos + 1 : instruction.size());

    // Extract destination register
    size_t commaPos = instruction.find(',');
    destReg = instruction.substr(0, commaPos);
    instruction.erase(0, commaPos != string::npos ? commaPos + 2 : instruction.size());

    // Extract first source register
    size_t separatorPos = instruction.find_first_of(",(");
    srcReg1 = instruction.substr(0, separatorPos);
    instruction.erase(0, separatorPos != string::npos ? separatorPos + 1 : instruction.size());

    // Extract second source register
    size_t closingBracketPos = instruction.find(')');
    srcReg2 = instruction.substr(0, closingBracketPos);
    instruction.erase(0, closingBracketPos != string::npos ? closingBracketPos + 1 : instruction.size());

    // Ensure register names are two characters long
    if (!destReg.empty() && destReg.size() == 1) destReg += '0';
    if (!srcReg1.empty() && srcReg1.size() == 1) srcReg1 += '0';
    if (!srcReg2.empty() && srcReg2.size() == 1) srcReg2 += '0';

    // Parse different operations
    if (opCode == "LOAD") {
        parsedOutput = {1, destReg[1] - '0', atoi(srcReg1.c_str()), srcReg2[1] - '0'};
    } else if (opCode == "STORE") {
        parsedOutput = {2, atoi(srcReg1.c_str()), srcReg2[1] - '0', destReg[1] - '0'};
    } else if (opCode == "BNE") {
        parsedOutput = {3, destReg[1] - '0', srcReg1[1] - '0', atoi(srcReg2.c_str()) / 2};
    } else if (opCode == "CALL") {
        parsedOutput = {4, 0, 0, destReg[1] - '0'};
    } else if (opCode == "RET") {
        parsedOutput = {5, 0, 0, 0};
    } else if (opCode == "ADD") {
        parsedOutput = {6, destReg[1] - '0', srcReg1[1] - '0', srcReg2[1] - '0'};
    } else if (opCode == "ADDI") {
        parsedOutput = {7, destReg[1] - '0', srcReg1[1] - '0', atoi(srcReg2.c_str())};
    } else if (opCode == "NAND") {
        parsedOutput = {8, destReg[1] - '0', srcReg1[1] - '0', srcReg2[1] - '0'}; 
    }else if (opCode == "DIV") {
        parsedOutput = {9, destReg[1] - '0', srcReg1[1] - '0', srcReg2[1] - '0'};
    } else {
        cout << "\nWrong input!\n";
    }
    
    
    
    
    return parsedOutput;
}

vector<vector<ReservationStation>> allReservationStations(7);

int available_RS(vector<ReservationStation> stations) {
    for (int i = 0; i < stations.size(); i++) {
        if (stations[i].isBusy == 0) {
            return i;
        }
    }
    return -1;
}

vector<ReservationStation>& OP_CODE_ASSOCIATED_stations(int inst) {
    // operation codes are as follows:
    // LOAD --> 1, STORE --> 2, BNE --> 3, CALL --> 4, RET --> 5, ADD --> 6, ADDI --> 7, NAND --> 8, DIV --> 9
    
    switch (inst) {
    case 1: return allReservationStations[0];  //LOAD
    case 2: return allReservationStations[1];  //STORE
    case 3: return allReservationStations[2];  //BNE
    case 4: return allReservationStations[3];  //CALL/RET
    case 5: return allReservationStations[4];  //ADD/ADDI
    case 6: return allReservationStations[5];  //NAND
    case 7: return allReservationStations[6];  //DIV
        default: return allReservationStations[0];
    }
}


void simulateTomasulo(int& pc_start, vector<string>& instrucitons, vector<vector<int>>& instrucitons_vals, 
                      vector<int>& mem, vector<vector<ReservationStation>>& allReservationStations, 
                      vector<vector<int>>& table, vector<pair<string, int>>& registers_ready_or_not, 
                      vector<int>& registers, vector<pair<ReservationStation*, bool>>& write_queue, 
                      queue<ReservationStation*>& load_store_queue, int& clock_cycle, bool& not_complete, 
                      int& i, bool& br, bool& jmp, int& return_position, int& branch_miss, 
                      int& branch_predict) {
    	while (not_complete && clock_cycle < 200) {
	    
	    // Issuing stage 
		if (i < instrucitons_vals.size() && !jmp) { 
			vector<ReservationStation> & stations = OP_CODE_ASSOCIATED_stations(instrucitons_vals[i][0]);
			int RS_num = available_RS(stations);
			if (RS_num != -1) {
			
				//rs1
				if (stations[RS_num].Qj.first != "-1" && stations[RS_num].station_name!="LOAD_unit" && stations[RS_num].station_name != "CALL/RET_unit"  && registers_ready_or_not[instrucitons_vals[i][2]].first != "0") { 
					stations[RS_num].Qj = make_pair(registers_ready_or_not[instrucitons_vals[i][2]].first, registers_ready_or_not[instrucitons_vals[i][2]].second); 
				}
				else 
				{	
					if (stations[RS_num].station_name != "LOAD_unit" )
					{
						stations[RS_num].Vj = registers[instrucitons_vals[i][2]];
						stations[RS_num].Qj = { "0",0 };
					}
					else stations[RS_num].Qj = { "0",0 };
				}
				//rs2
				if (stations[RS_num].Qk.first != "-1"  &&stations[RS_num].station_name != "BNE_unit"&&instrucitons_vals[i][0] != 5&&instrucitons_vals[i][0]!=7&& registers_ready_or_not[instrucitons_vals[i][3]].first != "0") { 
					stations[RS_num].Qk = make_pair(registers_ready_or_not[instrucitons_vals[i][3]].first, registers_ready_or_not[instrucitons_vals[i][3]].second); 
				}
				else {
					if (stations[RS_num].station_name != "BNE_unit"&&instrucitons_vals[i][0] != 5 && instrucitons_vals[i][0] != 7)
					{
						stations[RS_num].Vk = registers[instrucitons_vals[i][3]];
						stations[RS_num].Qk = { "0",0 };
					}
					else stations[RS_num].Qk = { "0",0 };
				}

				if (stations[RS_num].station_name=="LOAD_unit") { 
					stations[RS_num].A = instrucitons_vals[i][2];
				}
				if (stations[RS_num].station_name == "STORE_unit")
				{
					stations[RS_num].A = instrucitons_vals[i][1];
				}
				if (instrucitons_vals[i][0] == 4 || instrucitons_vals[i][0] == 5) 
				{
					jmp = true;
				}

				if (instrucitons_vals[i][0] == 3)  
				{
					br = true;
				}

				stations[RS_num].isBusy = true;
				stations[RS_num].stage = 1;
				stations[RS_num].inst_num = i;
				stations[RS_num].branch_predict = (br&&stations[RS_num].station_name!= "BNE_unit");
				table.push_back({ clock_cycle,0,0,i });
				stations[RS_num].tablePosition = table.size() - 1;
				registers_ready_or_not[instrucitons_vals[i][1]].first = stations[RS_num].station_name;
				registers_ready_or_not[instrucitons_vals[i][1]].second = stations[RS_num].station_num;
				write_queue.push_back(make_pair(&stations[RS_num], 0));
				if (stations[RS_num].station_name == "LOAD_unit" || stations[RS_num].station_name == "STORE_unit") load_store_queue.push(&stations[RS_num]);
				if(!jmp) i++;
			}
		}

        // Executing Stage 
		for (int j = 0; j < allReservationStations.size(); j++) {
			for (int k = 0; k < allReservationStations[j].size(); k++) {

				int inst = allReservationStations[j][k].inst_num;

				if (allReservationStations[j][k].stage == 1 && allReservationStations[j][k].isBusy == true ) {

					if (instrucitons_vals[inst][0] == 1  && (&allReservationStations[j][k]==load_store_queue.front()) ) {

						if (allReservationStations[j][k].Qj.first == "0") {  
							allReservationStations[j][k].stage = 2;
							allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
							allReservationStations[j][k].A += allReservationStations[j][k].Vj;
						}

					}
					else if (instrucitons_vals[inst][0] == 2 && (&allReservationStations[j][k] == load_store_queue.front())) {

						if (allReservationStations[j][k].Qj.first == "0" && allReservationStations[j][k].Qk.first == "0") {  
							allReservationStations[j][k].stage = 2;
							allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
							allReservationStations[j][k].A += allReservationStations[j][k].Vj;
						}

					}
					else if (allReservationStations[j][k].Qj.first == "0" && allReservationStations[j][k].Qk.first == "0" &&!(instrucitons_vals[inst][0] == 1 || instrucitons_vals[inst][0] == 2)) {
						allReservationStations[j][k].stage = 2;
						allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
					}

				}
				else if (allReservationStations[j][k].stage == 2 && allReservationStations[j][k].counter > 1) {
					allReservationStations[j][k].counter--;
				}
				else if (allReservationStations[j][k].stage == 2 && allReservationStations[j][k].counter == 1 && !allReservationStations[j][k].branch_predict) { 
					table[allReservationStations[j][k].tablePosition][1] = clock_cycle;
					allReservationStations[j][k].counter--;
					
				}
				else if (allReservationStations[j][k].stage == 2 && allReservationStations[j][k].counter == 0) {
					for (int wq = 0; wq < write_queue.size(); wq++)         
					{
						if (write_queue[wq].first == &allReservationStations[j][k])
						{
							write_queue[wq].second = true;
							break;
						}
						allReservationStations[j][k].stage = 3;
					}

				}
			}

		}

		// Writing Stage 
		for (int wq = 0; wq < write_queue.size(); wq++) {
			int valu;
			if (write_queue[wq].second)
			{

				write_queue[wq].first->stage = 0;
				int inst = write_queue[wq].first->inst_num;
				write_queue[wq].first->isBusy = false;
				table[write_queue[wq].first->tablePosition][2] = clock_cycle;
				if ((registers_ready_or_not[instrucitons_vals[inst][1]].first == write_queue[wq].first->station_name) && (registers_ready_or_not[instrucitons_vals[inst][1]].second == write_queue[wq].first->station_num)) {
					registers_ready_or_not[instrucitons_vals[inst][1]].first = "0";
					registers_ready_or_not[instrucitons_vals[inst][1]].second = 0;
				}
				switch (instrucitons_vals[inst][0]) {
				case 1: //LOAD
					if (instrucitons_vals[inst][1] != 0)
					{
						valu = registers[instrucitons_vals[inst][1]] = mem[write_queue[wq].first->A];
					}
					else {
						valu = registers[instrucitons_vals[inst][1]] = 0;
					}
					load_store_queue.pop();
					if (!load_store_queue.empty() && load_store_queue.front()->Qj.first == "0" && load_store_queue.front()->station_name == "LOAD_unit")
					{
						load_store_queue.front()->stage = 2;
						load_store_queue.front()->counter = load_store_queue.front()->exe_cycles;
						load_store_queue.front()->A += load_store_queue.front()->Vj;
					}
					else if (!load_store_queue.empty() && load_store_queue.front()->Qk.first == "0" && load_store_queue.front()->Qj.first == "0" && load_store_queue.front()->station_name == "LOAD_unit")
					{
						load_store_queue.front()->stage = 2;
						load_store_queue.front()->counter = load_store_queue.front()->exe_cycles;
						load_store_queue.front()->A += load_store_queue.front()->Vj;
					}
					break;
				case 2: //STORE
					mem[write_queue[wq].first->A] = registers[instrucitons_vals[inst][3]];
					load_store_queue.pop();
					if (!load_store_queue.empty() && load_store_queue.front()->Qj.first == "0" && load_store_queue.front()->station_name=="LOAD_unit")
					{
						load_store_queue.front()->stage = 2;
						load_store_queue.front()->counter = load_store_queue.front()->exe_cycles;
						load_store_queue.front()->A += load_store_queue.front()->Vj;
					}else if (!load_store_queue.empty() && load_store_queue.front()->Qk.first == "0" && load_store_queue.front()->Qj.first == "0" && load_store_queue.front()->station_name == "LOAD_unit")
					{
						load_store_queue.front()->stage = 2;
						load_store_queue.front()->counter = load_store_queue.front()->exe_cycles;
						load_store_queue.front()->A += load_store_queue.front()->Vj;
					}
					break;
				case 3: //BNE
					br = false;
					if (registers[instrucitons_vals[inst][1]] != registers[instrucitons_vals[inst][2]]) {
						branch_miss++;
						for (int wqq = wq+1; wqq < write_queue.size(); wqq++)
						{
							int wqq_inst = write_queue[wqq].first->inst_num;
							write_queue[wqq].first->isBusy = false;
							write_queue[wqq].first->stage = 0;
							if ((registers_ready_or_not[instrucitons_vals[wqq_inst][1]].first == write_queue[wqq].first->station_name) && (registers_ready_or_not[instrucitons_vals[wqq_inst][1]].second == write_queue[wqq].first->station_num)) {
								registers_ready_or_not[instrucitons_vals[wqq_inst][1]].first = "0";
								registers_ready_or_not[instrucitons_vals[wqq_inst][1]].second = 0;
							}
							
						}
						i = inst + instrucitons_vals[inst][3];
					
					}
					else if (registers[instrucitons_vals[inst][1]] == registers[instrucitons_vals[inst][2]]) {
						branch_predict++;
						for (int wqq = wq+1; wqq < write_queue.size(); wqq++)
							write_queue[wqq].first->branch_predict = false;
						
					}
					break;
				case 4: //CALL
					return_position = i+1 ;
					i = (instrucitons_vals[inst][3]-pc_start)/2;
					jmp = false;
					break;
				case 5: //RET
					i = return_position;
					jmp = false;
					break;
				case 6: //ADD
					if (instrucitons_vals[inst][1] != 0)
					{
						valu = registers[instrucitons_vals[inst][1]] = registers[instrucitons_vals[inst][2]] + registers[instrucitons_vals[inst][3]];
					}
					else valu = registers[instrucitons_vals[inst][1]] = 0;
					 break;
				case 7: //ADDI 
					if (instrucitons_vals[inst][1] != 0)
					{
					valu = registers[instrucitons_vals[inst][1]] = registers[instrucitons_vals[inst][2]] + instrucitons_vals[inst][3];
					}
					else valu = registers[instrucitons_vals[inst][1]] = 0;
					break;
				case 8: //NAND
					if (instrucitons_vals[inst][1] != 0)
					{
						valu = registers[instrucitons_vals[inst][1]] = ~(registers[instrucitons_vals[inst][2]] & registers[instrucitons_vals[inst][3]]);
					}
					else valu = registers[instrucitons_vals[inst][1]] = 0;
					 break;
				case 9: //DIV
					if (instrucitons_vals[inst][1] != 0)
					{
					if (registers[instrucitons_vals[inst][3]] != 0) valu = registers[instrucitons_vals[inst][1]] = registers[instrucitons_vals[inst][2]] / registers[instrucitons_vals[inst][3]];
					}
					else valu = registers[instrucitons_vals[inst][1]] = 0;
					break;
				}

				for (int j = 0; j < allReservationStations.size(); j++) {
					for (int k = 0; k < allReservationStations[j].size(); k++) {
						if (write_queue[wq].first->station_name != "BNE_unit"&&write_queue[wq].first->station_name != "STORE_unit" && (allReservationStations[j][k].Qj.first == write_queue[wq].first->station_name) && (allReservationStations[j][k].Qj.second == (write_queue[wq].first)->station_num)) {
							allReservationStations[j][k].Qj.first = "0";
							allReservationStations[j][k].Vj = valu;
							
							if (instrucitons_vals[inst][0] == 1 || instrucitons_vals[inst][0] == 2) {

								if (allReservationStations[j][k].Qj.first == "0") {  
									allReservationStations[j][k].stage = 2;
									allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
									allReservationStations[j][k].A += allReservationStations[j][k].Vj;
								}

							}
							else if (allReservationStations[j][k].Qj.first == "0" && allReservationStations[j][k].Qk.first == "0") {
								allReservationStations[j][k].stage = 2;
								allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
							}
						}

						if (write_queue[wq].first->station_name != "BNE_unit"&&write_queue[wq].first->station_name!="STORE_unit"&&(allReservationStations[j][k].Qk.first == write_queue[wq].first->station_name )&& (allReservationStations[j][k].Qk.second == (write_queue[wq].first)->station_num)) {
							allReservationStations[j][k].Qk.first = "0";
							allReservationStations[j][k].Vk = valu;
							
							if (instrucitons_vals[inst][0] == 1 || instrucitons_vals[inst][0] == 2) {

								if (allReservationStations[j][k].Qj.first == "0") {  
									allReservationStations[j][k].stage = 2;
									allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
									allReservationStations[j][k].A += allReservationStations[j][k].Vj;
								}

							}
							else if (allReservationStations[j][k].Qj.first == "0" && allReservationStations[j][k].Qk.first == "0") {
								allReservationStations[j][k].stage = 2;
								allReservationStations[j][k].counter = allReservationStations[j][k].exe_cycles;
							}
						}
					
					}
				}

				write_queue.erase(write_queue.begin() + wq);
				break;
			}
		}

		bool complete = true;
		for (int r = 0; r < table.size(); r++) { 
			if (table[r][2] == 0&& table[r][1] >0) {
				complete = false;
			}
		}
		
		not_complete = !complete||(i<=instrucitons_vals.size());
		clock_cycle++;
	}
}



int main() {
    int pc_start;
    vector<string> instrucitons;
    vector<vector<int>> instrucitons_vals;
    cout << "First instruction address\n";
    cin >> pc_start;
    cout << "How many instructions?\n";
    int input_counter;
    cin >> input_counter;
    cin.clear();
    cin.ignore(1000, '\n');
    cout << "Input all the instructions, seperat them by new lines\n";
    
    while (input_counter > 0)
    {
        string i_s;
        getline(cin, i_s);
        instrucitons.push_back(i_s);
        instrucitons_vals.push_back(parseInstruction(i_s));
        input_counter--;
    }
    vector<int> mem(64000, 0);
    cout << "How many data memory items?\n";
    cin >> input_counter;
    cout << "Input the address of each item then its value\n";
    while (input_counter > 0)
    {
        int m, n;
        cin >> m >> n;
        mem[m] = n;
        input_counter--;
    }
    
    
    vector<int> num_Stations(7);
    vector<int> cycles(7);

        cout << "Enter Number of reservation stations for LOAD, STORE, BNE, CALL/RET, ADD/ADDI, NAND, DIV and units respectively: \n";
        for (int &val : num_Stations) {
            cin >> val;
        }

        cout << "Enter Number of cycles for each of the units respectively: \n";
        for (int &val : cycles) {
            cin >> val;
        }

        int maxStations = *max_element(num_Stations.begin(), num_Stations.end());

        for (int i = 0; i < maxStations; i++) {
            if (i < num_Stations[0])
                allReservationStations[0].push_back({0, "NOP", 0, -1, {"0", -1}, {"0", -1}, 0, cycles[0], 0, "LOAD_unit", i, 0, 0, 0});
            if (i < num_Stations[1])
                allReservationStations[1].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, 0, cycles[1], 0, "STORE_unit", i, 0, 0, 0});
            if (i < num_Stations[2])
                allReservationStations[2].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, 0, cycles[2], 0, "BNE_unit", i, 0, 0, 0});
            if (i < num_Stations[3])
                allReservationStations[3].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, 0, cycles[3], 0, "CALL/RET_unit", i, 0, 0, 0});
            if (i < num_Stations[4])
                allReservationStations[4].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, -1, cycles[4], 0, "ADD/ADDI_unit", i, 0, 0, 0});
            if (i < num_Stations[5])
                allReservationStations[5].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, -1, cycles[5], 0, "NAND_unit", i, 0, 0, 0});
            if (i < num_Stations[6])
                allReservationStations[5].push_back({0, "NOP", 0, 0, {"0", -1}, {"0", -1}, -1, cycles[6], 0, "DIV_unit", i, 0, 0, 0});
        }

        vector<vector<int>> table(0, vector<int>(4, 0));

        vector<pair<string, int>> registers_ready_or_not(1000, { "0",0 }); 
        vector<int> registers(8, 0); 

        vector<std::unique_ptr<ReservationStation>> inst_Rs(instrucitons_vals.size());
        vector<pair<ReservationStation*, bool>> write_queue;
        int pc = 200;
        bool br, jmp;
        br = jmp = false;
        bool not_complete = true;
        int i = 0;
        int clock_cycle = 1;
        int return_position;
        int branch_miss=0;
        int branch_predict = 0;
        queue<ReservationStation*> load_store_queue;
        
    
        simulateTomasulo(pc_start, instrucitons, instrucitons_vals, mem, allReservationStations, table, 
                     registers_ready_or_not, registers, write_queue, load_store_queue, 
                     clock_cycle, not_complete, i, br, jmp, return_position, branch_miss, 
                     branch_predict);

    int max_clk = 0;
    
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" ; 
    cout << std::left << std::setw(20) << "Instruction" << std::setw(10) << "Issue" << std::setw(10) << "Execute" << std::setw(10) << "Write" << endl;

    for (int i = 0; i < table.size(); i++) {
    if (table[i][2] == 0) {
        table.erase(table.begin() + i);
        i--; 
    }
    }

    for (int i = 0; i < table.size(); i++) {
    if (max_clk < table[i][2]) max_clk = table[i][2];

    cout << std::left << std::setw(20) << instrucitons[table[i][3]]; // Instruction
    for (int j = 0; j < table[i].size() - 1; j++) {
        cout << std::setw(10) << table[i][j]; // Issue, Execute, Write
    }
    cout << endl;
    }

    cout << "clock cycles required: " << max_clk << endl;
    cout << "CPI: " << double(max_clk) / table.size()<<endl;
    double branch_miss_p = double(branch_miss) / (branch_miss + branch_predict) ;
    cout << "Branch miss % = " << branch_miss_p*100<<endl;
    
    return 0; 
}
