entity 2band2_or2_lut is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z  : out BIT;
    );
end entity;

architecture 2band2_or2_lut_arch of 2band2_or2_lut is
begin

    process (abcd) is
    begin
        case abcd of
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
end architecture 2band2_or2_lut_arch; 