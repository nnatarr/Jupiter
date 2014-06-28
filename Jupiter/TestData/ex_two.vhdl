entity f is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture f_a of f is
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
            when "1110" => z <= "1";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture f_a; 

entity s is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture s_a of s is
begin
    process (abcd) is
    begin
        case abcd of
            when "0000" => z <= "0";
            when "0001" => z <= "0";
            when "0010" => z <= "0";
            when "0011" => z <= "0";
            when "0100" => z <= "1";
            when "0101" => z <= "1";
            when "0110" => z <= "1";
            when "0111" => z <= "1";
            when "1000" => z <= "0";
            when "1001" => z <= "0";
            when "1010" => z <= "0";
            when "1011" => z <= "0";
            when "1100" => z <= "1";
            when "1101" => z <= "1";
            when "1110" => z <= "1";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture s_a; 

entity t is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture t_a of t is
begin
    process (abcd) is
    begin
        case abcd of
            when "0000" => z <= "0";
            when "0001" => z <= "0";
            when "0010" => z <= "0";
            when "0011" => z <= "1";
            when "0100" => z <= "0";
            when "0101" => z <= "0";
            when "0110" => z <= "0";
            when "0111" => z <= "1";
            when "1000" => z <= "0";
            when "1001" => z <= "0";
            when "1010" => z <= "0";
            when "1011" => z <= "1";
            when "1100" => z <= "0";
            when "1101" => z <= "0";
            when "1110" => z <= "0";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture t_a; 

entity fo is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture fo_a of fo is
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
            when "1001" => z <= "1";
            when "1010" => z <= "0";
            when "1011" => z <= "1";
            when "1100" => z <= "0";
            when "1101" => z <= "1";
            when "1110" => z <= "0";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture fo_a; 

entity r is
    port (
        abcd : in std_ulogic_vector(3 downto 0);
        z : out BIT;
    );
end entity;

architecture r_a of r is
begin
    process (abcd) is
    begin
        case abcd of
            when "0000" => z <= "0";
            when "0001" => z <= "1";
            when "0010" => z <= "1";
            when "0011" => z <= "1";
            when "0100" => z <= "0";
            when "0101" => z <= "1";
            when "0110" => z <= "0";
            when "0111" => z <= "1";
            when "1000" => z <= "1";
            when "1001" => z <= "1";
            when "1010" => z <= "1";
            when "1011" => z <= "1";
            when "1100" => z <= "1";
            when "1101" => z <= "1";
            when "1110" => z <= "1";
            when "1111" => z <= "1";
        end case;
    end process;
end architecture r_a; 

entity result_entity is
    port (
        a,b,c,d : in BIT;
        z : out BIT;
    );
end entity;

architecture result_entity_arch of result_entity is
    component f port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;
    component s port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;
    component t port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;
    component fo port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;
    component r port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;

    signal s_one, s_two, s_three, s_fourth: BIT;
begin
    f port map (a, b, c, 0, s_one);
    s port map (0, 0, 0, d, s_two);
    t port map (0, 0, c, d, s_three);
    fo port map (a, 0, 0, d, s_fourth);
    r port map (s_one, s_two, s_three, s_fourth, z);
end architecture result_entity_arch; 