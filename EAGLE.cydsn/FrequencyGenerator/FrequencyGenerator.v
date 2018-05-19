
//`#start header` -- edit after this line, do not edit this line
// ========================================
//
// Copyright YOUR COMPANY, THE YEAR
// All Rights Reserved
// UNPUBLISHED, LICENSED SOFTWARE.
//
// CONFIDENTIAL AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF your company.
//
// ========================================
`include "cypress.v"
//`#end` -- edit above this line, do not edit this line
// Generated on 04/12/2018 at 21:20
// Component: FrequencyGenerator
module FrequencyGenerator (
	output  pwmX,
	output  pwmY,
	input  [7:0] accuWordY,
	input   clk,
	input   reset_n
);
	parameter accuOffset = 4400;
	parameter accuSize = 16;
	parameter accuWordX = 18;

//`#start body` -- edit after this line, do not edit this line
reg [7:0] countX;
reg [accuSize-1:0] countY;

//        Your code goes here
always @ (posedge clk or negedge reset_n)
begin
    if( reset_n == 1'b0 )
    begin
        countX <= 0;
        countY <= 0;
    end
    else
    begin
        countX <= countX + accuWordX;
        countY <= countY + accuWordY + accuOffset;
    end
end

assign pwmX = (reset_n && countX[7]);
assign pwmY = (reset_n && countY[accuSize-1]);

//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
