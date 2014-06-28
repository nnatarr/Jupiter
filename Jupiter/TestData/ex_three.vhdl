entity f is
    port (
        abc : in std_ulogic_vector(2 downto 0);
        z : out BIT;
    );
end entity;

architecture f_a of f is
begin
    process (abc) is
    begin
        case abc of
            when "000" => z <= "1";
            when "001" => z <= "1";
            when "010" => z <= "1";
            when "011" => z <= "1";
            when "100" => z <= "1";
            when "101" => z <= "1";
            when "110" => z <= "1";
            when "111" => z <= "0";
        end case;
    end process;
end architecture f_a; 