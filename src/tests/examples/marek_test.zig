// Program 3: Prace s retezci a vestavenymi funkcemi
const ifj = @import("ifj24.zig");
// Hlavni funkce
pub fn main() void {
const str1 = ifj.string("Toto je text v programu jazyka IFJ24");
var str2 = ifj.string(", ktery jeste trochu obohatime");
str2 = ifj.concat(str1, str2);
ifj. write(str1); ifj . write("\n");
ifj .write(str2); ifj.write("\n");
ifj.write("Zadejte serazenou posloupnost malych pismen a-h:\n");
var newInput = ifj.readstr();
var all: []u8 = ifj.string("");

while (newInput) |inpOK| {
const abcdefgh = ifj.string("abcdefgh");
const strcmpResult = ifj.strcmp(inpOK, abcdefgh);
if (strcmpResult == 0) {

} if(0) {
	const y = 5;
}

const x = 5;

}

}