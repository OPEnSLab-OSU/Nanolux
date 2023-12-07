import style from './style.css';
import {useState} from "preact/hooks";

const Save_Entry = ({

    name,
    idx

}) => {
    const [value, setValue] = useState(0);

    const valueChanged = (event) => {
        setValue(event.target.value);
    };

    return (
        <div class='entry'>
            <tr>
                <th>{name}</th>
                <th>
                    <button>Load Pattern</button>
                </th>
                <th>
                    <button border-radius='5px'>Save Pattern</button>
                </th>
            </tr>
        </div>
    );
}

export default Save_Entry;
