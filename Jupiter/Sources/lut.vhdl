-- library ieee;
-- use ieee.std_logic_1164.all;
-- use ieee.numeric_std.all;

entity Number_of_Ones is
    port (
        --- mapped 3=a, 2=b, 1=c, 0=d
        abcd : in std_ulogic_vector(3 downto 0);
        -- mapped x=2, y=1, z=0
        xyz  : out std_ulogic_vector(2 downto 0);
    );
end entity;

architecture any of Number_of_Ones is
begin

    process (abcd) is
    begin
        case abcd of
        --ABCD|XYZ
        when "0000" => xyz <= "000";
        when "0001" => xyz <= "001";
        when "0010" => xyz <= "001";
        when "0011" => xyz <= "010";
        when "0100" => xyz <= "011";
        when "0101" => xyz <= "010";
        when "0110" => xyz <= "010";
        when "0111" => xyz <= "011";
        when "1000" => xyz <= "001";
        when "1001" => xyz <= "010";
        when "1010" => xyz <= "010";
        when "1011" => xyz <= "011";
        when "1100" => xyz <= "010";
        when "1101" => xyz <= "011";
        when "1110" => xyz <= "011";
        when "1111" => xyz <= "100";
        end case;
    end process;
end architecture; 