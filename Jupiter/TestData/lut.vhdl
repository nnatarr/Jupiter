entity 2band2_or2_lut is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture 2band2_or2_lut_arch of 2band2_or2_lut is
begin
    process (abcd) is
    begin
        case abcd of
            when "0000" => z <= "0";
            when "0001" => z <= "0";
            when "0010" => z <= "0";
            when "0011" => z <= "0";
            when "0100" => z <= "0";
            when "0101" => z <= "0";
            when "0110" => z <= "0";
            when "0111" => z <= "0";
            when "1000" => z <= "0";
            when "1001" => z <= "0";
            when "1010" => z <= "0";
            when "1011" => z <= "0";
            when "1100" => z <= "0";
            when "1101" => z <= "0";
            when "1110" => z <= "0";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture 2band2_or2_lut_arch; 