entity lut0 is
    port (
        input: in std_ulogic_vector(3 downto 0);
        output: out BIT;
    );
end entity;

architecture lut0_arch of lut0 is
begin
    process (input) is
    begin
        case input of
            when "0000" => output <= "0";
            when "0001" => output <= "0";
            when "0010" => output <= "0";
            when "0011" => output <= "0";
            when "0100" => output <= "0";
            when "0101" => output <= "0";
            when "0110" => output <= "0";
            when "0111" => output <= "1";
            when "1000" => output <= "0";
            when "1001" => output <= "0";
            when "1010" => output <= "0";
            when "1011" => output <= "0";
            when "1100" => output <= "0";
            when "1101" => output <= "0";
            when "1110" => output <= "0";
            when "1111" => output <= "1";
        end case;
    end process;
end architecture lut0_arch;

entity lut1 is
    port (
        input: in std_ulogic_vector(3 downto 0);
        output: out BIT;
    );
end entity;

architecture lut1_arch of lut1 is
begin
    process (input) is
    begin
        case input of
            when "0000" => output <= "0";
            when "0001" => output <= "0";
            when "0010" => output <= "0";
            when "0011" => output <= "1";
            when "0100" => output <= "0";
            when "0101" => output <= "1";
            when "0110" => output <= "0";
            when "0111" => output <= "1";
            when "1000" => output <= "0";
            when "1001" => output <= "0";
            when "1010" => output <= "0";
            when "1011" => output <= "1";
            when "1100" => output <= "0";
            when "1101" => output <= "1";
            when "1110" => output <= "0";
            when "1111" => output <= "1";
        end case;
    end process;
end architecture lut1_arch;

entity lut4 is
    port (
        input: in std_ulogic_vector(3 downto 0);
        output: out BIT;
    );
end entity;

architecture lut4_arch of lut4 is
begin
    process (input) is
    begin
        case input of
            when "0000" => output <= "0";
            when "0001" => output <= "1";
            when "0010" => output <= "1";
            when "0011" => output <= "1";
            when "0100" => output <= "1";
            when "0101" => output <= "1";
            when "0110" => output <= "1";
            when "0111" => output <= "1";
            when "1000" => output <= "1";
            when "1001" => output <= "1";
            when "1010" => output <= "1";
            when "1011" => output <= "1";
            when "1100" => output <= "1";
            when "1101" => output <= "1";
            when "1110" => output <= "1";
            when "1111" => output <= "1";
        end case;
    end process;
end architecture lut4_arch;


entity main_entity is
    port (
        x0, x1, x2, x3: in BIT;
        output: out BIT;
    );
end entity;

architecture main_entity_arch of main_entity is
    component lut0 port(tx0, tx1, tx2, tx3: in BIT; txout: out BIT);
    end component;
    component lut1 port(tx0, tx1, tx2, tx3: in BIT; txout: out BIT);
    end component;
    component lut4 port(tx0, tx1, tx2, tx3: in BIT; txout: out BIT);
    end component;

    signal s0, s1: BIT;
begin
    lut0 port map(0, x0, x1, x2, s0);
    lut1 port map(0, x0, x2, x3, s1);
    lut4 port map(s0, s1, 0, 0, output);
end architecture main_entity_arch;